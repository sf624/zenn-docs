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

## NASAによる定義

https://shemesh.larc.nasa.gov/fm/papers/Hayhurst-2001-tm210876-MCDC.pdf



##

![](/images/various-coverage/image.png)
*NASAの文献p.7より抜粋*





以上の事情から、この記事ではC0/C1/C2を以下のように定義する。

### C0カバレッジ

ステートメントカバレッジとする。

```cpp
void foo(bool a, bool b) {
    if (a && b) {
        volatile int i = 0;
    }
}

void main () {
    foo(true, true); // C0 coverage
                     // not C1 coverage
}
```

### C1カバレッジ

判断文カバレッジとする。

```cpp
void foo(bool a, bool b) {
    if (a && b) {
        volatile int i = 0;
    }
}

void main () {
    foo(true, true);
    foo(true, false); // C1 coverage
                      // not C2 coverage
}
```

### C2カバレッジ

単純条件カバレッジとする。ただし、短絡評価を考慮せずに各条件が真・偽の2種類が入力されるだけで良しとするのか、短絡評価を考慮して評価されなかった条件についてはカバレッジの対象外とするかで結果が異なる。この記事においては、前者を"C2 (without short circuit)"、後者を"C2 (with short circuit)"とする。後者のほうがより厳しい条件である。

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
}
```

## カバレッジの順位

C1カバレッジが取れている場合、すべての判断分岐上の命令が実行されることになるため、C0カバレッジが取れていることになる。また、C2を短絡評価を考慮した単純条件カバレッジとして定義する場合、

C0 (Statement Coverage) = LLVM Line Coverage <= C1 <= C2 (with short circuit) = LLVM Branch Coverage < MCDC

## Clangのカバレッジ



## 補足：各社によるC0/C1/C2の定義

用語についても各社で揺らぎがあったり、同じ用語であっても言葉の定義が異なるということがあったため、NASAの[文献](https://shemesh.larc.nasa.gov/fm/papers/Hayhurst-2001-tm210876-MCDC.pdf)で用いられている用語とその定義で統一して記載する。（例えば、NASAでいう"Multiple Condition Coverage"を、VECTORでは"Condition Coverage"と呼称していた）

C0とC1についてはほとんど差異が見られないが、C2についてはかなりの定義の違いがあった。

| 出典 | C0 | C1 | C2 |
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
