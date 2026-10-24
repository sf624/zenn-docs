---
title: "C++の仮想テーブルの仕組み"
emoji: "💽"
type: "tech"
topics:
  - "cpp"
  - "c"
  - "低レイヤ"
  - "国際標準規格"
published: false
---

C++では、ポリモーフィズムはどのようにして実現されているのだろうか。ここで以下のような、仮想関数を持つ基底クラスおよび派生クラスを考える。このとき、virtual関数の呼び出しがどのように実現されているか、実際のアセンブラも確認しながら見てみよう。

```cpp
#include <cstdint>
#include <typeinfo>

// 基底クラス
struct Base {
  Base() { b_data = 123; }
  virtual ~Base() { b_data = 234; }
  virtual void foo() { b_data = 345; }
  void bar() { b_data = 456; } // non-virtual関数
  int32_t b_data;
};

struct Base2 {
    Base2() { b2_data = -123; }
    virtual ~Base2() { b2_data = -234; }
    virtual void buz() { b2_data = -345; } 
    void bar() { b2_data = -456; } // non-virtual関数
    int32_t b2_data;
};

// 派生クラス
struct Derived : public Base, public Base2 {
  Derived() { d_data = 567; }
  virtual ~Derived() override { d_data = 678; }
  virtual void foo() override { d_data = 789; }
  virtual void buz() override { d_data = 890; }
  void bar() { d_data = 901; } // non-virtual関数

  int32_t d_data;
};

// ポリモーフィックなデータ・メソッドアクセス
int32_t get_b_data(Base * b) {
    return b->b_data; // データメンバーへのアクセス
}

int32_t get_b2_data(Base2 * b2) {
    return b2->b2_data;
}

int32_t get_d_data(Derived * d) {
    return d->d_data;
}

void call_foo(Base * b) {
    b->foo(); // virtualな関数呼び出し (BaseとDerivedで共有)
}

void call_buz(Base2 * b2) {
    b2->buz(); // virtualな関数呼び出し (Base2とDerivedで共有)
}

void call_bar(Base * b) {
    b->bar(); // non-virtualな関数呼び出し
}

void get_type_name(Base * b) {
    typeid(*b).name();
}

void* get_raw_address(Base2 * b2) {
    return dynamic_cast<void*>(b2);
}

// オブジェクトの構築方法
// スタック
void construct_on_stack() {
    Base base_auto{};
    Base2 base2_auto{};
    Derived derived_auto{};

    call_foo(&base_auto);
    call_foo(&derived_auto);
    call_buz(&base2_auto);
    call_buz(&derived_auto);
    get_type_name(&derived_auto);
}

//　ヒープ
void construct_on_heap() {
    Base * base_heap = new Base{};
    Derived * derived_heap = new Derived{};

    delete base_heap;
    delete derived_heap;
}
```

gcc 15.2でのコンパイル結果は以下の通りになる。

https://godbolt.org/z/orbYh78xr

まず、早速ではあるがポリモーフィズムを支えているvtable([virtual method table](https://ja.wikipedia.org/wiki/%E4%BB%AE%E6%83%B3%E9%96%A2%E6%95%B0%E3%83%86%E3%83%BC%E3%83%96%E3%83%AB))に着目しよう。

```asm
vtable for Derived:
        .quad   0
        .quad   typeinfo for Derived
        .quad   Derived::~Derived() [complete object destructor]
        .quad   Derived::~Derived() [deleting destructor]
        .quad   Derived::foo()
        .quad   Derived::buz()
        .quad   -16
        .quad   typeinfo for Derived
        .quad   non-virtual thunk to Derived::~Derived() [complete object destructor]
        .quad   non-virtual thunk to Derived::~Derived() [deleting destructor]
        .quad   non-virtual thunk to Derived::buz()
vtable for Base2:
        .quad   0
        .quad   typeinfo for Base2
        .quad   Base2::~Base2() [complete object destructor]
        .quad   Base2::~Base2() [deleting destructor]
        .quad   Base2::buz()
vtable for Base:
        .quad   0
        .quad   typeinfo for Base
        .quad   Base::~Base() [complete object destructor]
        .quad   Base::~Base() [deleting destructor]
        .quad   Base::foo()
```

`.quad`は8バイトデータを意味し、x86-64は64bitシステムであるため、関数ポインタが8byteのサイズで格納されている。

0バイト目に格納されるのは多重継承時に使用するオフセット値であり、後に説明する。

8バイト目に格納されるのが実行時型情報（RTTI）である。RTTIは、実際の型(dynamic type)が何であるかの情報を提供してくれるものであり、`typeid`による型情報の取得や、`dynamic_cast`による基底クラスから派生クラス方向へのダウンキャストを実現することができる。ダウンキャストは、`reinterpret_cast`によっても実現ができるが、RTTIを参照せずにダウンキャストするため実際の型でないものに変換してUndefined Behaviorとする危険性があり推奨されない。またダウンキャストそのものもアンチパターンとされることが多く、RTTIを不要とするプログラムであれば、`-fno-rtti`などのコンパイラオプションを指定することで本機能を無効とすることができる。

そして16バイト目以降に、クラスのvirtual関数へのポインタが登録されており、今回の例でいえばvirtualなデストラクタおよびメソッド（`foo`や`buz`）が登録される。C言語風に書き直すと以下のような構造体が静的領域に配置されている。

```c
struct BaseVtable {
    ptrdiff_t top_offest = 0;
    TypeInfo* rtti = &base_typeinfo;
    void (*destructor)(void*) = &Base_complete_destructor;
    void (*deleter)(void*) = &Base_delete_destructor;
    void (*foo)(void*) = &Base::foo;
};

static VtableForBase1 vtable_for_base1{};

struct VtableForBase2 {
    ptrdiff_t top_offest = 0;
    TypeInfo* rtti =  &base2_typeinfo;
    void (*destructor)(void*) = &Base2_complete_destructor;
    void (*deleter)(void*) = &Base2_delete_destructor;
    void (*buz)(void*) = &Base2::buz;
};

static VtableForBase2 vtable_for_base2{};

struct VtableForDerived {
    // Baseとの共通部分
    ptrdiff_t top_offest = 0;       // +0
    TypeInfo* rtti = &derived_typeinfo;             // +8
    void (*destructor)(void*) = &Dervived_complete_destructor;  // +16
    void (*deleter)(void*) = &Derived_delete_destructor;     // +24
    void (*foo)(void*) = &Dervived::foo;         // +32

    // Base2用のメソッド
    void (*buz)(void*) = &Dervived::buz;         // +40

    // Base2として扱った場合に使用する部分(Thunk経由)
    ptrdiff_t top_offest = -16;         // +48
    TypeInfo* rtti = &derived_typeinfo;         // +56
    void (*destructor)(void*)
      = &Thunk_to_Dervived_complete_destructor;         // +64
    void (*deleter)(void*)
      = &Thunk_to_Dervived_delete_destructor;         // +72
    void (*buz)(void*)
      = &Thunk_to_Dervived::buz;         // +80
};

static VtableForDerived vtable_for_derived{};
```

多重継承をしている`Derived`のvtableがいささか混みあっているため説明する。ポリモーフィズムを実現するためには、`Dervied`は`Base`クラスとしても`Base2`クラスとしても見なせるようにする必要がある。つまり、`Derived`のVtableは、`Base`のVTableと見なせる部分と`Base2`のVTableと見なせる部分の両方を持ち合わせている必要があるため、二つのvtableを連結したような構造となっている。

このvtableがどのようにしてポリモーフィズムを実現しているか説明する。例えば、`call_foo`関数は、与えられた`Base`オブジェクトへポインタから、実際の型(dynamic type)に応じた適切な`foo`関数、すなわち`Base::foo()`か`Derived::foo()`のいずれかを適切に呼び出す必要がある。ここで重要なのは、`call_foo`関数からすると与えられたオブジェクトの実際の型がどちらかなのかは分からなくても（agnostic）正しく動作する必要があるということである。（このようなポリモーフィズムにおいてはRTTIは使われない）

```cpp
void call_foo(Base * base) {
    base->foo(); // bの実体がDerivedなのかBaseなのかcall_fooからは分からない
}
```

実は、`base`には上記のvtableへのポインタが格納されており、それによって適切なvirtual関数を呼び出すことができるようになっている。つまり、以下のような呼び出しを実際には行っている。（`foo`に`base`が渡されているのは、`this`ポインタをクラスメソッドに渡すため）

```c
void call_foo(Base * base) {
    VTable* vtable = base; // baseはvtableへのポインタと等しい
    vtable->foo(base); // vtableを経由して、vtableに登録されたfoo関数を呼び出す
}
```

では、BaseやDerived構造体のデータ構造はどのようになっていなければならないだろうか。

vtableへのポインタは実際の型にagnosticにアクセスできなければならない。また、基底クラスに存在するデータメンバーは派生クラスからもアクセス可能であることから、やはりこれも実際の型にagnosticにアクセスできなければならない。これが意味するところは、**派生関係にあるクラスについてはvtableを含めた共通部分については構造体の先頭から同じ位置に配置されていなければならないということである。**

vtableは、基本的にBaseやDerivedを構成する構造体の先頭に配置される。それ以降は、基底クラスのデータメンバーから順番に配置されることになる。従って、BaseやDerivedの構造体やポリモーフィックにクラスを扱う関数の呼び出しの様子は、C言語風に表現すると以下のようになる。

```c
struct Base {
    Vtable * vtable; // vtableへのポインタ (+0byte)
    int32_t b_data;      // (+8byte)
};

struct Base2 {
    Vtable * vtable; // vtableへのポインタ (+0byte)
    int32_t b2_data;     // (+8byte)
};

struct Derived {
    // Baseとの共通部分
    Vtable * vtable; // vtableへのポインタ (+0byte)
    int32_t b_data;      // Baseと同じ位置に配置される (+8byte)

    // パディング
    uin32_t padding;  // (+12byte)

    // Base2との共通部分　(+16byte)
    Vtable * vtable; // b2用のvtableへのポインタ  (+16byte)
    int32_t b2_data;     // (+24byte)

    // Dervied固有部分
    int32_t d_data;      // (+28byte)
};

int32_t get_b_data(Base * base) {
    return b->b_data; // 共通するデータメンバーは同じ位置に配置されているため、基底クラスだと見なしてアクセスすればよい
}

void call_foo(Base * base) {
    VTable* vtable = base;
    vtable->foo(); // virtual関数は、vtableを通してアクセスする
}

void call_bar(Base * base) {
    Base::bar(base); // non-virtualな関数はコンパイル時に呼び出す関数が決定されるため、vtableを経由しない
}
```

しかし、ここまでの説明であれば`Derived`にある`vtable`は`Base`の`vtable`と共通なのであって、`Base2`の`vtable`とは異なるため、`Base2`としてのポリモーフィズムは実現されないように思われる。そこで、コンパイラは`Derived`を`Base2`として関数に渡す場合は、実は`Derived`のポインタを`Base2`の`vtable`の先頭に一致するように変更してから関数呼び出しをしている。

実際に、以下のアセンブリを見てみると、`Dervied`が`Base2`として渡される`call_buz`関数の呼び出しの時だけ、`Derived`のポインタ値に16が可算されて渡されている。この「16」バイトとは、上記の`Derived`構造体において、先頭から`Base2`としてのvtableへのアドレスの距離である。このように、**ポリモーフィックタイプのポインタは基底クラスに変換された場合には、必ずしも同一のアドレス値を保持するとは限らない。**

```asm
construct_on_stack():
        push    rbp
        mov     rbp, rsp
        push    rbx
        sub     rsp, 72
        lea     rax, [rbp-32] ; rbp-32が&base_auto
        mov     rdi, rax
        call    Base::Base() [complete object constructor]
        lea     rax, [rbp-48] ; rbp-48が&base2_auto
        mov     rdi, rax
        call    Base2::Base2() [complete object constructor]
        lea     rax, [rbp-80] ; rbp-80が&derived_auto
        mov     rdi, rax
        call    Derived::Derived() [complete object constructor]
        lea     rax, [rbp-32]
        mov     rdi, rax
        call    call_foo(Base*) ; base_autoのcall_foo呼び出しは、rbp-32=&base_autoが渡されている
        lea     rax, [rbp-80]
        mov     rdi, rax
        call    call_foo(Base*)　; derived_autoのcall_foo呼び出しは、rbp-80=&derived_autoが渡されている
        lea     rax, [rbp-48]
        mov     rdi, rax
        call    call_buz(Base2*) ; base2_autoのcall_buz呼び出しは、rbp-48=&base2_autoが渡されている
        lea     rax, [rbp-80]
        add     rax, 16 ; 16がrbp-80=&derived_autoに加算されている
        mov     rdi, rax
        call    call_buz(Base2*) ; derived_autoのcall_buz呼び出しは、rbp-80+16=&derived_auto+16が渡されている
        ...
```


次に、コンストラクタについてアセンブリを確認する。

```asm
Base::Base() [base object constructor]:
        push    rbp
        mov     rbp, rsp
        mov     QWORD PTR [rbp-8], rdi
        mov     edx, OFFSET FLAT:vtable for Base+16
        mov     rax, QWORD PTR [rbp-8]
        mov     QWORD PTR [rax], rdx    ; &Base(=vtable)にBaseのVtableの+16バイト位置を登録
        mov     rax, QWORD PTR [rbp-8]
        mov     DWORD PTR [rax+8], 123 ; Base.b_dataを初期化
        nop
        pop     rbp
        ret
        .set    Base::Base() [complete object constructor],Base::Base() [base object constructor]

Base2::Base2() [base object constructor]:
        push    rbp
        mov     rbp, rsp
        mov     QWORD PTR [rbp-8], rdi
        mov     edx, OFFSET FLAT:vtable for Base2+16
        mov     rax, QWORD PTR [rbp-8]
        mov     QWORD PTR [rax], rdx     ; &Base2(=vtable)にBase2のVtableの+16バイト位置を登録
        mov     rax, QWORD PTR [rbp-8]
        mov     DWORD PTR [rax+8], -123 ; Base2.b2_dataを初期化
        nop
        pop     rbp
        ret
        .set    Base2::Base2() [complete object constructor],Base2::Base2() [base object constructor]


Derived::Derived() [base object constructor]:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 16
        mov     QWORD PTR [rbp-8], rdi
        mov     rax, QWORD PTR [rbp-8]
        mov     rdi, rax
        call    Base::Base() [base object constructor] ; &Dervied(=vtable=&Base)をBaseコンストラクタで初期化
        mov     rax, QWORD PTR [rbp-8]
        add     rax, 16
        mov     rdi, rax
        call    Base2::Base2() [base object constructor] ; &Dervied+16(=第2vtable=&Base2)をBase2コンストラクタで初期化
        mov     edx, OFFSET FLAT:vtable for Derived+16
        mov     rax, QWORD PTR [rbp-8]
        mov     QWORD PTR [rax], rdx ; &Dervied(=vtable=&Base)をDerived自身の第1vtable+16バイト位置で上書き
        mov     edx, OFFSET FLAT:vtable for Derived+64 
        mov     rax, QWORD PTR [rbp-8]
        mov     QWORD PTR [rax+16], rdx ; &Dervied+(=vtable=&Base)をDerived自身の第2vtable+16バイト位置で上書き
        mov     rax, QWORD PTR [rbp-8]
        mov     DWORD PTR [rax+28], 567 ; Derived.d_dataの初期化
        nop
        leave
        ret
        .set    Derived::Derived() [complete object constructor],Derived::Derived() [base object constructor]
```

`Base`や`Base2`のコンストラクタにおいては、vtableを設定後にデータメンバーを初期化している。なお、vtableのアドレスを格納する上で16byteを可算している。これは、Itaniami ABIでは、`top_to_offset`とRTTIの冒頭16バイト部分を飛ばした位置（最初のvirtual関数位置）で登録するという取り決めとなっているからである。

`Derived`コンストラクタについては、`Base`と`Base2`コンストラクタの呼び出して、`Base`と`Base2`相当部分を初期化したあとに、`Derived`用のvtableへの差し替えと`Derived`専用のデータメンバーの初期化をしている。C++では、クラスの構築は基底クラス側から順に実施されるとされているため、この仕様に従っていることが分かる。

つまり、C言語風に表現すると以下のようになる。

```c
void Base::Base(Base * base) {
    base->vtable = &vtable_for_base;
    base->b_data = 123;
}

void Base2::Base2(Base * base2) {
    base2->vtable = &vtable_for_base2;
    base2->b2_data = -123;
}

void Derived::Derived(Derived * derived) {
    Base::Base(derived);
    Base2::Base2(derived + 16); // = Base2::Base2((Base2*)derived);
    derived->vtable = &vtable_for_derived; // vtableをbase用のものからderived用のものに上書きする
    derived->vtable2 = &vtable2_for_derived;
    derived->d_data = 567;           // b_dataについてはBaseコンストラクタで設定済のため不要
}
```

次にデストラクタを確認する。

```asm
Base::~Base() [base object destructor]:
        push    rbp
        mov     rbp, rsp
        mov     QWORD PTR [rbp-8], rdi
        mov     edx, OFFSET FLAT:vtable for Base+16
        mov     rax, QWORD PTR [rbp-8]
        mov     QWORD PTR [rax], rdx
        mov     rax, QWORD PTR [rbp-8]
        mov     DWORD PTR [rax+8], -42
        nop
        pop     rbp
        ret
        .set    Base::~Base() [complete object destructor],Base::~Base() [base object destructor]

Derived::~Derived() [base object destructor]:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 16
        mov     QWORD PTR [rbp-8], rdi
        mov     edx, OFFSET FLAT:vtable for Derived+16
        mov     rax, QWORD PTR [rbp-8]
        mov     QWORD PTR [rax], rdx
        mov     rax, QWORD PTR [rbp-8]
        mov     DWORD PTR [rax+12], -334
        mov     rax, QWORD PTR [rbp-8]
        mov     rdi, rax
        call    Base::~Base() [base object destructor]
        nop
        leave
        ret
        .set    Derived::~Derived() [complete object destructor],Derived::~Derived() [base object destructor]

```

Baseにおいてはvtableを自身のものに差し替えたあとにデストラクタ専用の処理（ここでは`b_data=-42`）、Derivedにおいてはvtableを自身のものに差し替えたあとにデストラクタ専用処理（ここでは`d_data=-334`）を実施してそのあとにBaseデストラクタを呼び出している。C++においては、オブジェクトの破棄は派生クラス側から順に実施されるとしているため、やはりこの仕様に沿っていることが分かる。

それぞれが自身のデストラクタ呼び出し時に、自身のvtableを設定しているのは、デストラクト時に仮想関数呼び出しが行われる可能性があるからである。

以上から、デストラクタ処理をC言語風に書き直すと以下のようになっている。

```c
void Base::~Base(Base * base) {
    base->vtable = &vtable_for_base;
    base->b_data = -42;
}

void Derived::~Derived(Derived * derived) {
    derived->vtable = &vtable_for_derived;
    derived->d_data = -334;
    Base::~Base((Base *)derived);
}
```
