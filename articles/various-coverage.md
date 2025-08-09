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

ソフトウェアのテスト品質を図る指標の一つとして、コードカバレッジがある。

## C0/C1/C2って何なのか

まず、そもそもの話として「C0」「C1」「C2」の定義がどこから来ているのかから調査したが、意外なことに、これらの用語は、調べた限りにおいては、ISOなどの国際標準規格などでは定義されておらず、企業によってもその定義に揺らぎがあり、この記事の最後に参考として各社の定義を掲載した。

**特に、C2カバレッジについては「単純条件網羅」とするものと「複合条件網羅」としているものの2種類の流派がある。** これらは全く異なるカバレッジであることから、これらの用語を使うときは必ず定義を確認する必要がある。

また、C2カバレッジを単純条件網羅と定義した場合でも、C/C++においては「[短絡評価](https://ja.wikibooks.org/wiki/%E3%83%97%E3%83%AD%E3%82%B0%E3%83%A9%E3%83%9F%E3%83%B3%E3%82%B0/%E7%9F%AD%E7%B5%A1%E8%A9%95%E4%BE%A1#:~:text=%E7%9F%AD%E7%B5%A1%E8%A9%95%E4%BE%A1%EF%BC%88Short%2DCircuit%20Evaluation,%E6%80%A7%E3%82%92%E5%90%91%E4%B8%8A%E3%81%95%E3%81%9B%E3%81%BE%E3%81%99%E3%80%82)」が存在するため、短絡評価によって実際には評価されたなった条件をどう扱うかでまた違いが出てくる。

その定義に揺らぎがある理由については、以下の記事が参考となった。

> 現在、カバレッジの説明の際によく使われるC0やC1と呼ばれる基準は1975年頃にEdward Millerが提案したものです。ただ、Millerはこれらの基準の定義を何回か変更しているので、論文の発表時期により少しずつ定義が異なっています。

http://a-lifelong-tester.cocolog-nifty.com/blog/2011/12/4-277e.html

## ISTQBによる定義

https://jstqb.jp/dl/JSTQB_CTAL-TTA_Syllabus_version2024.v4.0.J01.pdf

## NASAによる定義

https://shemesh.larc.nasa.gov/fm/papers/Hayhurst-2001-tm210876-MCDC.pdf



##

![](/images/various-coverage/image.png)
*NASAの文献p.7より抜粋*





以上の事情から、この記事ではC0/C1/C2を以下のように定義する。

### C0カバレッジ

Statement Coverageと定義する。これは、C/C++で定義されるところの各statementが一度でも実行されたかどうかを指す。


```cpp
void foo(bool a) {
    if (a) {
        volatile int i = 0;
    }
}

void main () {
    foo(true); // C0 coverage
               // not C1 coverage
               // not C2 coverage
}
```

多くの場合、後述するDecision Coverageが取れていれば、Statement Coverageも取れていることになる。しかしながら、厳密にはDecision Coverage（更にはMC/DCやMCCにも）には包含されない。なぜなら、`return`文のあとに到達不可能なStatementがある場合があるからである。しかしながら、このようなプログラムは保守性の観点から一般にはコーディングルール等の導入によって排除されるものであり、普通のコーディングにおいてはDecision CoverageがStatement Coverageを包含すると考えてよい。

```cpp
void foo(bool a) {
    if (a) {
        volatile int i = 0;
    }
    return;
    volatile int j = 1; // UNREACHABLE!
}

void main () {
    foo(true);
    foo(false); // Decision, Condition, Condition/Decision, MC/DC, MCC coverage
                // but not Statement Coverage!
}

```

### C1カバレッジ

Decision Coverageと定義する。これは、C/C++においては[`if`文、`switch`文、](https://timsong-cpp.github.io/cppwp/n4950/stmt.select)[`for`文、`while`文](https://timsong-cpp.github.io/cppwp/n4950/stmt.iter)のレベルで定義される[`condition`](https://timsong-cpp.github.io/cppwp/n4950/stmt.pre#nt:condition)について、それぞれの`true`と`false`が1回以上成立することと定義する。

例えば、以下の例であれば`a && b`が`if`文の`condition`にあたり、`a`と`b`の個別の真偽については問わない。

```cpp
void foo(bool a, bool b) {
    if (a && b) {
        volatile int i = 0;
    }
}

void main () {
    foo(true, true);
    foo(true, false); // C1 coverage
                      // also C0 coverage
                      // not C2 coverage
}
```

### C2カバレッジ

Condition Coverageとする。これは、C/C++においては、`bool`と評価される`expression`の最小単位のそれぞれについて、`true`と`false`が少なくとも1回以上成立することと定義する。

なお、C/C++では短絡評価が存在するため、ここでいう「成立」には以下の3種類の定義のされ方が考えられる。

1. 各条件式への入力値によって**独立して**決定する
2. C/C++が短絡評価が行わないと仮定した場合に、実際に各条件式が`true`と`false`に評価されるかどうか
3. 短絡評価を考慮して、実際に各条件式が`true`と`false`に評価されるかどうか

この中で、Condition Coverageとして巷でよく使われる定義は1.のようである（NASAも同じであった）。従って、1.による定義を本記事でも採用することとする。つまり、以下はC2カバレッジが網羅されることになる。

```cpp
void foo(bool a, bool b) {
    if (a && b) { // a==falseの場合、短絡評価によりbは評価されないため、
                  // b==trueは実際には評価されない
        volatile int i = 0;
    }
}

void main () {
    foo(true, false);
    foo(false, true);  // C2 coverage (without short circuit),
                       // not C0 coverage
                       // not C1 coverage
                       // not C2 coverage (with short circuit)
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

以上を考慮すると、あらゆるC/C++プログラムに対して、カバレッジが定義できるのは3.の方式ではあるものの、調べた限りではこれをCondition Coverageと定義している文献は見つからなかった。当然、この場合は以下のようにカバレッジの条件が厳しくなり、またC1カバレッジ(Decision coverage)も自動的に取れることになる。

```cpp
void foo(bool a, bool b) {
    if (a && b) {
        volatile int i = 0;
    }
}

void main () {
    foo(true, true);
    foo(true, false);
    foo(false, false): // C2 coverage (with short circuit)
                       // C1 coverage
                       // C0 coverage
}
```

## カバレッジの順位

C1カバレッジが取れている場合、すべての判断分岐上の命令が実行されることになるため、C0カバレッジが取れていることになる。また、C2を短絡評価を考慮した単純条件カバレッジとして定義する場合、

C0 (Statement Coverage) = LLVM Line Coverage <= C1 <= C2 (with short circuit) = LLVM Branch Coverage < MCDC

## Clangのカバレッジ



## 補足：各社によるC0/C1/C2の定義

用語についても各社で揺らぎがあったり、同じ用語であっても言葉の定義が異なるということがあったため、NASAの[文献](https://shemesh.larc.nasa.gov/fm/papers/Hayhurst-2001-tm210876-MCDC.pdf)で用いられている用語とその定義で統一して記載する。（例えば、NASAでいう"Multiple Condition Coverage"を、VECTORでは"Condition Coverage"と呼称していた）

C0とC1についてはほとんど差異が見られないが、C2の定義についてはかなりの違いがあり、主にCondition Coverageとする流派とMuptiple Condition Coverage (MCC)とする流派の2つが目立った。前述したように、Condition CoverageであればMC/DCよりも遥かに弱いカバレッジであるのに対して、MCCはMC/DCよりも強いカバレッジであり、レベル感が全く異なる。

| 出典 | C0 Coverage | C1 Coverage | C2 Coverage |
| - | :-: | :-: | :-: |
| [テクマトリクス](https://www.techmatrix.co.jp/t/quality/coverage.html) | Statement Coverage | Decision Coverage | Condition Coverage |
| [Sky](https://www.skygroup.jp/tech-blog/article/610/) | Statement Coverage | Decision Coverage | Path Coverage?[^sky] |
| [SHIFT](https://service.shiftinc.jp/column/4547/) | Statement Coverage | Decision Coverage | **Multiple Condition Coverage** |
| [NRI Netcom](https://tech.nri-net.com/entry/coverage_c0_c1_c2_mcc) | Statement Coverage | Decision Coverage | Condition Coverage |
| [ガイオ・テクノロジー](https://www.gaio.co.jp/gaioclub/glossary_blog05/#col02-1) | Statement Coverage | Decision Coverage | Condition Coverage?[^gaio] |
| [QBOOK](https://www.qbook.jp/column/632.html) | Statement Coverage | Decision Coverage | **Multiple Condition Coverage** |
| [computex](https://www.computex.co.jp/products/technology/coverage/index.htm) | Statement Coverage | ?[^computex] | ?[^computex] |
| [AGEST](https://agest.co.jp/column/2021-09-24/) | Statement Coverage | Decision Coverage | Condition Coverage |
| [MONOist](https://monoist.itmedia.co.jp/mn/articles/1610/20/news009.html) | Statement Coverage | Deicision Coverage | **Multiple Condition Coverage** |
| [VECTOR](https://www.vector.com/jp/ja/know-how/vj-columns/vj-software-testing/vj-columns220330/#c289232) | Statement Coverage  | Deicision Coverage | **Multiple Condition Coverage** |

[^sky]: C2の定義を「条件式間のコードパスの全ての組み合わせ」としており、NASAは言及していないが、いわゆる"Path Coverage"であると判断した。

[^gaio]: 出典の説明が曖昧であり、Condition Coverageかどうかの判断が難しかった。

[^computex]: 出典には「分岐網羅」、「条件網羅」とそれぞれ記載があるが、説明文を見ると普通の定義のされ方と違うように見えることから?とした。
