---
title: "Passkey idiom (パスキー・イディオム)"
emoji: "🪪"
type: "tech" # tech: 技術記事 / idea: アイデア
topics: ["cpp"]
published: false
---

既に他の方が紹介されていますが、面白いなと思ったのでこちらでも紹介したいと思います。

https://yohhoy.hatenadiary.jp/entry/20240402/p1

Friendクラスを使って、他のクラスにプライベートメンバーを共有したいケースにおいて、共有するメンバーを限定できる方法です。例えば、以下のようなコードにおいて

```cpp
class F {
    friend class G;
    int data_;
    int data_really_private_; // こっちはGに公開したくない…
};

class G {
public:
    int get_f_data(const F & f) const {
        return f.data_;
    }
};
```

以下のように、クラス自身にしか自身で特殊化したクラスを作成できないPasskeyというクラスを用意することで、そのPasskeyを要求するようなパブリックメンバー関数を用意することでアクセス制御が可能になります。クラス自身でしか対応するPasskeyを作れないところが、ちょうど認証技術の"Passkey"と似ているのが面白いです。

```cpp
template <typename T>
class Passkey {
    friend T;
    explicit Passkey() = default; // TしかPasskey<T>を作れない
    // Note: C++20より前ではexplicit指定しないと、
    // 集成体初期化の仕様により他のクラスがPasskeyを作成できてしまう
};

class G; // 前方宣言

class F {
    int data_;
    int data_really_private_; // こっちはGでも見れない
public:
    int get_data(Passkey<G>) const { // GのPasskeyを要求する
         return data_; 
    } 
};

class G {
    int data_;
public:
    int get_f_data(const F & f) const {
        // 自身のPasskey<G>は作成できる
        return f.get_data(Passkey<G>{});
    }
};

class H {
public:
    int get_f_data(const F & f) const {
        // G以外は、Passkey<G>を作れない
        return f.get_data(Passkey<G>{}); // コンパイルエラー

    }
};
```

C++20以降は、集成体初期化の仕様が変更されたことから、explicit指定がなくてもよくなりました。また、Passkeyを作成するクラスが特定されているのであれば、特にテンプレートにする必要は必ずしもないです。

```cpp
class G;

// Passkeyを使うクラスが決まっているならテンプレートである必要はない
class GPasskey {
    friend G;
    GPasskey() = default; // C++20以降はexplicit指定は不要
};

class F {
    int data_;
    int data_really_private_;
public:
    int get_data(GPasskey) const { 
        return data_; 
    }
};

class G {
    int data_;
public:
    int get_f_data(const F & f) const {
        return f.get_data({}); // explicitでないので{}だけでOK
    }
};
```
