---
title: "C0/C1/C2カバレッジとClangカバレッジの関係"
emoji: "🙆"
type: "tech" # tech: 技術記事 / idea: アイデア
topics: ["c", "cpp", "coverage", "gcc", "clang"]
published: false
---

## この記事を読むと…

- C0/C1/C2カバレッジとは何かが分かる
- 各社によるC2カバレッジの定義に揺らぎがあることが分かる
- Clangのsource-based coverageがどのカバレッジに対応するのかが分かる

## はじめに

ソフトウェアのテスト品質を図る指標の一つとして、コードカバレッジがある。コードカバレッジには分岐の網羅性や命令文の網羅性といったように様々な種類のものがあり、日本ではよく「C0」、「C1」、「C2」といった名称のカバレッジが登場する。この記事では、C0/C1/C2がそもそもどういうカバレッジであるかを説明し、その後にClangのsouce-based coverageで取得できるカバレッジとの対応関係について解説する。

## C0/C1/C2って何なのか

まず、そもそもの話として「C0」「C1」「C2」の定義がどこから来ているのかから調査してみたが、これらの用語は調べた限りにおいては、ISOなどの国際標準規格などでは定義されておらず、企業によってもその定義に揺らぎがあることが分かった。この記事の最後に参考として各社がどのようにC0/C1/C2を定義していたかを掲載する。

**特に、C2カバレッジについては「単純条件網羅」とするものと「複合条件網羅」としているものの2種類の流派がある。** これらは全く異なるカバレッジであることから、C2という用語を使うときは必ず定義を確認する必要がある。

なぜこのように定義に揺らぎがあるのか調べたところ、以下の記事が参考となった。どうやら当時考案した人が、何回かその定義を変えたことにより、定義に曖昧性があるまま世の中に広まってしまったということのようだ。

> 現在、カバレッジの説明の際によく使われるC0やC1と呼ばれる基準は1975年頃にEdward Millerが提案したものです。ただ、Millerはこれらの基準の定義を何回か変更しているので、論文の発表時期により少しずつ定義が異なっています。

http://a-lifelong-tester.cocolog-nifty.com/blog/2011/12/4-277e.html

## NASAによるカバレッジの定義

C0/C1/C2という用語の定義には各業界で揺らぎがあり得ることが分かったので、もう少ししっかりと定義が統一されている名称のカバレッジをまず確認して、それとC0/C1/C2とを結びつけることにする。今回は、参照先としてNASAの文献を用いた。

https://shemesh.larc.nasa.gov/fm/papers/Hayhurst-2001-tm210876-MCDC.pdf

### Statement Coverage (命令カバレッジ)

実行可能な命令（executable statement）のそれぞれが少なくとも1回以上実行されることを指す。

C/C++においては、例えばif文の条件部分(expression)はC/C++でいうところの"statement"ではないが、「実行」可能な単位ではあるためこれも母数に含めるとするのが安全側であると思われる。平たくいえば、すべての行が一度以上実行されることを意味する。


```cpp
void foo(bool a) {
    if (a) {
        volatile int i = 0;
    }
}

void main () {
    foo(true); // Statement Coverage
               // not Decision Coverage
               // not Condition Coverage
}
```

### Branch Coverage (分岐カバレッジ)

if文などのstatementの分岐点において、真偽のどちらのパスも通過することを指す。


```cpp
void foo(bool a, bool b) {
    if (a && b) {
        volatile int i = 0;
    }
}

void main () {
    foo(true, true);
    foo(true, false); // Branch Coverage
                      // also Decision Coverage
                      // also Statement Coverage
                      // not Condition Coverage
}
```

### Decision Coverage (判断カバレッジ)

今後、"Condition"と"Decision"という2つのカバレッジに関する用語が出てくるので、まずその定義を確認する。NASAの上記の文献が引用しているDO-178Bによれば、

> Condition – A Boolean expression containing no Boolean operators.

> Decision – A Boolean expression composed of conditions and zero or more Boolean operators. A decision without a Boolean operator is a condition. If a condition appears more than once in a decision, each occurrence is a distinct condition. 

となっており、和訳すれば

- Condition: boolean演算子のないboolean式のこと (例:`true`)

- Deicision: 1個以上のConditionと0個以上のboolean演算子から構成されるboolean式のこと。boolean演算子を含まないDeicisionは（Conditionの定義からして）Conditionである。あるConditionがDeicision内に複数登場する場合、それぞれが独立したConditionとして区別される。

ということになる。

「あるConditionがDeicision内に複数登場する場合、それぞれが独立したConditionとして区別される。」という部分には注意が必要である。例えば、

```cpp
(A && B) || (A || C)
```

というDecisionがある場合、1つ目のAと2つ目のAは"Condition"という観点では**区別される**。

Decision Coverageは、このように定義される"Deicision"に対してDO-178Bにおいて、

> Decision Coverage - Every point of entry and exit in the program has been invoked at least once and every decision in the program has taken on all possible outcomes at least once.

と定義される。

`true`と`false`がそれぞれ少なくとも1回以上成立することを意味する。例えば、以下の例であれば`a && b`が`if`文の"Decision"にあたり、`a`と`b`の個別の真偽については問わない。


```cpp
void foo(bool a, bool b) {
    if (a && b) {
        volatile int i = 0;
    }
}

void main () {
    foo(true, true);
    foo(true, false); // Decision Coverage
                      // also Statement Coverage
                      // not Condition Coverage
}
```

なお、Decisionは必ずしもif文などの分岐を伴う文の条件である必要はなく、代入文などについても適用される。例えば、上記の例を下記のように書き換えた場合、真偽値を中継する`c = a && b`において`a && b`がDecisionとなり、カバレッジの母数として考慮される。

```cpp
void foo(bool a, bool b) {
    bool c = a && b; // <- This is also a Decision
    if (c) {
        volatile int i = 0;
    }
}

void main () {
    foo(true, true);
    foo(true, false); // Decision Coverage
                      // also Statement Coverage
                      // not Condition Coverage
}
```

**ここが単純な分岐網羅を考えるBranch Coverageとの大きな違いである。** 例えば、以下のようなコードを考えると、`d`は`true`しか取っていないのでDecision Coverageは成立していない。しかしながら、`if`文の分岐については真偽の両方のパスが成立するため、Branch Coverageは成立する。

```cpp
void foo(bool a, bool b, bool c) {
    bool d = a && b;
    if (c && d) {
        volatile int i = 0;
    }
}

void main () {
    foo(true, true, true);
    foo(true, true, false); // Branch Coverage
                            // not Decision Coverage
                            // also Statement Coverage
                            // not Condition Coverage
}
```

このようにDO-178Bが定義する"Decision"に従うと、Decisin Coverage (DC)はBranch Coverageと異なるのだが、一部の業界では"Decision"を"Branchpoint"として定義しており、結果として"Decision Coverage"と"Branch Coverage"を同一視する場合がある。このような事情については、以下の文献でよく説明されている。兎にも角にも、本文献においては上述したように、DO-178Bが定義する"Decision"に従って、Decision Coverageを定義する。

https://people.eecs.ku.edu/~saiedian/Teaching/814/Readings/structural-testing-mcdc.pdf

### Condition Coverage (条件カバレッジ)

前述した"Condition"に対して、それぞれの真偽が少なくとも1回以上成立することと定義する。

なお、C/C++では短絡評価が存在するため、ここでいう「成立」には以下の3種類の定義のされ方が考えられる。

1. 各条件式への入力値によって**独立して**決定する
2. C/C++が短絡評価が行わないと仮定した場合に、実際に各条件式が`true`と`false`に評価されるかどうか
3. 短絡評価を考慮して、実際に各条件式が`true`と`false`に評価されるかどうか

この中で、Condition Coverageとして巷でよく使われる定義は1.のようである（NASAも同じであった）。従って、1.による定義を本記事でも採用することとする。

```cpp
void foo(bool a, bool b) {
    if (a && b) { // a==falseの場合、短絡評価によりbは評価されないため、
                  // b==trueは実際には評価されない
        volatile int i = 0; // この行は実行されないため、Statement Coverageは取れない
    }
}

void main () {
    foo(true, false);
    foo(false, true);  // Condition Coverage
                       // not Statement Coverage
                       // not Decision Coverage
}
```

なお、今回のように1.の方式を採用する場合、以下のような先行する条件式の結果によって後続する条件式の結果が決まるような場合においてはカバレッジがどうなるかは定義されないことになる。（`x = a`は`x`よりも先に評価されるため、C/C++の言語規格上は合法なプログラムである）

```cpp
void foo(bool a) {
    bool x;
    if ((x = a) && x) { // x = aを評価するまで、xの真偽は決定しない
        ...
    }
}
```

また、これを考慮して2.の方式を採用しようとすると、以下のように短絡評価を考慮しないとUBになるプログラムに対して、カバレッジが定義できないことになる。

```cpp
void foo(bool * a) {
    if ((a != nullptr) && (*a)) { // aがnullptrである場合に右辺(*a)を評価するとUB
        ...
    }
}
```

以上を考慮すると、あらゆるC/C++プログラムに対して、カバレッジが定義できるのは3.の方式ではあるものの、調べた限りではこれをCondition Coverageと定義している文献は見つからなかった。当然、この場合は以下のようにカバレッジの条件が厳しくなり、またDecision coverageも自動的に取れることになる。

```cpp
void foo(bool a, bool b) {
    if (a && b) {
        volatile int i = 0;
    }
}

void main () {
    foo(true, true);
    foo(true, false);
    foo(false, false): // Condition Coverage with short circuit consideration
                       // Statement Coverage
                       // Decision Coverage
}
```

### Condition/Decision Coverage （条件/判断カバレッジ）

Condition CoverageとDecision Coverageの両方が成立することを指す。

```cpp
void foo(bool a, bool b) {
    if (a && b) {
        volatile int i = 0;
    }
}

void main () {
    foo(true, true);
    foo(false, false); // Condition/Decision Coverage
                       // Condition Coverage
                       // Statement Coverage
                       // Decision Coverage
}
```

### Modified Condition/Decision Coverage (修正条件/判断カバレッジ)

略して、MC/DCと呼ばれる。Conditionのそれぞれの真偽の変化が、それ単独でDecisionの真偽を変化させるようなテストケースの組み合わせが存在することであり、DO-178Bでは下記のように定義される。

> Modified Condition/Decision Coverage - Every point of entry and exit in the program has been invoked at least once, every condition in a decision in the program has taken all possible outcomes at least once, every decision in the program has taken all possible outcomes at least once, and each condition in a decision has been shown to independently affect that decision’s outcome. A condition is shown to independently affect a decision’s outcome by varying just that condition while holding fixed all other possible conditions.

例えば、先ほどの「Condition/Decision Coverage」で提示した例においては、`(a, b) = {(true, true), (false, false)}`の組み合わせでは、

```cpp
void foo(bool a, bool b) {
    if (a && b) {
        volatile int i = 0;
    }
}

void main () {
    foo(true, true);
    foo(true, false);
    foo(false, true);  // Modified Condition/Decision Coverage
                       // Condition/Decision Coverage
                       // Condition Coverage
                       // Statement Coverage
                       // Decision Coverage
}
```

前述したように、「あるConditionがDeicision内に複数登場する場合、それぞれが独立したConditionとして区別される。」のであった。つまり、このような状況にあるDecisionについては、真偽を共有しているConditionについて一方のConditionを固定して他方のConditionの真偽を変化させることは不可能であるため、MC/DCを取るためにはそうならないように等価なものに書き換える必要がある。

```
# MC/DCのためには同じConditionが複数登場するDecisionは、
# そうならないように書き直さなければならない。
a && a --> a
a || !a --> true
(a && b) || (a && c) --> a && (b || c)
```

なお、MC/DCが取れていれば論理演算におけるロジックの誤りが全くないことを示すことにはならない。例えば、以下の例ではMC/DCが取れているものの、`a || b`が`a != b`であっても同じ結果を生み出すものの、`foo(true, true)`を実行した場合には結果が異なる。従って、このようなケースを含めて論理演算の誤りがないことを完璧にテストケースで示すには後述するMultiple Condition Coverageを取らなければいけない。

```cpp
void foo(bool a, bool b) {
    if (a != b) {
        volatile int i = 0;
    }
}

void main () {
    foo(false, false);
    foo(false, true);
    foo(true, false);  // Modified Condition/Decision Coverage
                       // Condition/Decision Coverage
                       // Condition Coverage
                       // Statement Coverage
                       // Decision Coverage
    // Above test case cannot distinguish `a or b` and `a not_eq b`.
}
```

### Multiple Condition Coverage

略して、MCCと呼ばれる。Conditionのそろぞれの真偽の組み合わせについて、すべての組み合わせについて実行されることである。つまり、Deicision内にConditionがN個あるのであれば、$2^N$個のテストケースが必要となる。テストケースが指数関数的に増大するため、複雑なDeicisionの場合は現実的でなくなることがある。そのため、品質が求められる業界であってもMCCではなくMC/DCまでが求められるケースが多い。

### 各種カバレッジまとめ

![](/images/various-coverage/image.png)
*NASAの文献p.7より抜粋*

大雑把に見れば、右側に行くほど強い。ただ、前述したサンプルコードでも分かるように、Deicision CovaregeとContion Coverageについては一方がもう片方を包含するという関係にはない。

更によく見ると、Deicision Coverage ~ Multiple Condition Coverageは必ずしもStatement Coverageを意味しない。なぜなら、例えば`return`文のあとに到達不可能なStatementを定義することが可能であるからだ。しかしながら、このようなプログラムは保守性の観点から一般にはコーディングルール等の導入によって排除されるものであり、普通のコーディングにおいてはDecision CoverageがStatement Coverageを包含すると考えてよい。

```cpp
void foo(bool a) {
    if (a) {
        volatile int i = 0;
    }
    return;
    volatile int j = 1; // UNREACHABLE!
}
```

## C0/C1/C2カバレッジの定義

以上の事情から、この記事ではC0/C1/C2を以下のように定義する。

### C0カバレッジ

Statement Coverageと定義する。

### C1カバレッジ

Decision Coverageと定義する。

### C2カバレッジ

Condition Coverageと定義する。

## Clangのカバレッジ

### Function Coverage

### Line Coverage

Statement Coverageである。

### Region Coverage

### (Clang) Branch Coverage

前述したBranch Coverageとは異なり、こちらは短絡評価を考慮したCondition Coverageである。

### MC/DC

Masking MC/DCである。

## 補足：各社によるC0/C1/C2の定義

用語についても各社で揺らぎがあったり、同じ用語であっても言葉の定義が異なるということがあったため、NASAの[文献](https://shemesh.larc.nasa.gov/fm/papers/Hayhurst-2001-tm210876-MCDC.pdf)で用いられている用語とその定義で統一して記載する。（例えば、NASAでいう"Multiple Condition Coverage"を、VECTORでは"Condition Coverage"と呼称していた）

C0とC1はほとんどの文献でそれぞれ"Statement Coverage"と"Branch Coverage"と定義しており差異が見られないが、C2の定義についてはかなりの違いがあり、主にCondition Coverageとする流派とMuptiple Condition Coverage (MCC)とする流派の2つが目立った。前述したように、Condition CoverageであればMC/DCよりも遥かに弱いカバレッジであるのに対して、MCCはMC/DCよりも強いカバレッジであり、レベル感が全く異なる。

| 出典 | C0 Coverage | C1 Coverage | C2 Coverage |
| - | :-: | :-: | :-: |
| [テクマトリクス](https://www.techmatrix.co.jp/t/quality/coverage.html) | Statement Coverage | Branch Coverage | Condition Coverage |
| [Sky](https://www.skygroup.jp/tech-blog/article/610/) | Statement Coverage | Branch Coverage | Path Coverage?[^sky] |
| [SHIFT](https://service.shiftinc.jp/column/4547/) | Statement Coverage | Branch Coverage[^branch-and-decision] | **Multiple Condition Coverage** |
| [NRI Netcom](https://tech.nri-net.com/entry/coverage_c0_c1_c2_mcc) | Statement Coverage | Branch Coverage | Condition Coverage |
| [ガイオ・テクノロジー](https://www.gaio.co.jp/gaioclub/glossary_blog05/#col02-1) | Statement Coverage | Branch Coverage | Condition Coverage?[^gaio] |
| [QBOOK](https://www.qbook.jp/column/632.html) | Statement Coverage | Branch Coverage[^branch-and-decision] | **Multiple Condition Coverage** |
| [computex](https://www.computex.co.jp/products/technology/coverage/index.htm) | アセンブラレベルでのStatement Coverage | ?[^computex] | アセンブラレベルでのBranch Coverage |
| [AGEST](https://agest.co.jp/column/2021-09-24/) | Statement Coverage | Branch Coverage[^branch-and-decision] | Condition Coverage |
| [MONOist](https://monoist.itmedia.co.jp/mn/articles/1610/20/news009.html) | Statement Coverage | Branch Coverage | **Multiple Condition Coverage** |
| [VECTOR](https://www.vector.com/jp/ja/know-how/vj-columns/vj-software-testing/vj-columns220330/#c289232) | Statement Coverage  | Branch Coverage | **Multiple Condition Coverage** |

[^branch-and-decision]: 元の文献にはDecision Coverageと書いているが、Branch CoverageとDecision Coverageを同一視しており、本記事で説明した"literal"なDecision Coverageとは定義が異なる。従って、"Branch Coverage"をここでは用語として用いる。

[^sky]: C2の定義を「条件式間のコードパスの全ての組み合わせ」としており、NASAは言及していないが、いわゆる"Path Coverage"であると判断した。

[^gaio]: 出典の説明が曖昧であり、Condition Coverageかどうかの判断が難しかった。

[^computex]: 出典には「分岐網羅」とあるが、その定義は分岐判定が真が成立したかどうかであり、真偽の両方は考慮しないとしている。

## その他参考文献

https://maskray.me/blog/2024-01-28-mc-dc-and-compiler-implementations


