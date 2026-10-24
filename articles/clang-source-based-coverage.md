---
title: "Clangのソースベースド(source-based)カバレッジ計測によるMC/DC測定"
emoji: "📔"
type: "tech" # tech: 技術記事 / idea: アイデア
topics: ["c", "cpp", "clang", "llvm", "coverage"]
published: true
published_at: "2025-07-28 07:00"
---

## この記事を読むと…

- Clangのsource basedカバレッジの計測・取得方法が分かります
- MC/DCのカバレッジ計測ができるようになります
- カバレッジ結果が具体的にどのように表示されるかが分かります

## はじめに

C/C++のカバレッジ計測手法についてはgcovが特に有名であるが、LLVMは独自に"source-based"カバレッジと呼ばれる手法を提供している。gcovでは、カバレッジ計測用のコード（instrument）がコンパイルの最終段階で挿入されるため最適化などの影響を受けやすい。一方で、"source-based"カバレッジではその名の通り、ソースコードレベルでinstrumentが挿入されるため[ほぼ最適化の影響を受けない](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html#impact-of-llvm-optimizations-on-coverage-reports)高精度なカバレッジ測定が可能となっている。

特に、LLVM 18.1.0からは[MC/DC](https://en.wikipedia.org/wiki/Modified_condition/decision_coverage)（修正条件／決定網羅）という種類のカバレッジが計測できるようになり、より精密で実用的なカバレッジ計測が可能となっている。そこでこの記事では、"source-based"カバレッジの基本的な計測方法を説明し、その計測結果を確認する。

今回は、以下のClangの"source based code coverage"の公式レファレンスをもとに解説した。

https://clang.llvm.org/docs/SourceBasedCodeCoverage.html

また、以下の記事も参考にさせていただいた。

https://qiita.com/joule/items/f9de29ceb1d78c5658d8

## LLVMのバージョンについて

現時点（2025年7月）で安定版の最新版であるLLVM 20.1.8を使用する。

```sh
$ clang++-20 --version
Ubuntu clang version 20.1.8 (++20250708082440+6fb913d3e2ec-1~exp1~20250708202457.136)
Target: x86_64-pc-linux-gnu
Thread model: posix
InstalledDir: /usr/lib/llvm-20/bin
```

インストールする場合、`llvm.sh`を用いるのが最も簡単である。([ref](https://apt.llvm.org/))

```sh
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 20
```

## その他のカバレッジ測定手法について

Clangには3つのカバレッジ測定手法があり、混同すると良くないので予め説明する。

- gcov: GCCが提供するカバレッジ測定。最もよく知られているカバレッジ測定手法。
- Sanitizer Coverage: 軽量カバレッジ。実行回数などのカウントはせず、到達したかどうかだけ測定する。
- Source based code coverage: LLVMが独自に提供するカバレッジ測定。gcovよりも詳細な測定が可能であり、最適化による影響を受けづらい。

このうち、今回説明するのは最後の"source based"カバレッジである。

特にgcovとの違いには注意を要する。GCC互換なClangもgcov仕様のカバレッジをサポートしているが、"source based"カバレッジとは別物であり、ネットで検索していて最もヒットしやすいのはこちらのカバレッジである。**`--coverage`オプションを指定した場合は、このgcov形式のカバレッジが計測される。**

```sh
# これらはgcov仕様のカバレッジを計測するコンパイル
g++ --coverage foo.cpp
clang++ --coverage foo.cpp
```

対して、今回説明するsource-basedカバレッジは以下のように**指定するオプションが異なる**。（なお、上記のオプションと組み合わせた場合は、gcovと"source-based"の両方によるカバレッジ計測が実施される。）

```sh
# これは"source based code coverage"仕様のカバレッジを計測するコンパイル
clang++ -fprofile-instr-generate -fcoverage-mapping foo.cpp
```

このあたり、事情がややこしく混同しやすいため、以下によくカバレッジ測定で出てくるコマンド・用語名とその意味を示す。名前が紛らわしいが、**`lcov`は`llvm-cov`とは別物で**、記事の最後に登場するHTMLのレポートは`lcov`の機能ではなく、`llvm-cov show`によるものである。

| 用語 | 役割 | 関連するファイル |
| - | - | - |
| `gcov` | GCCが提供するカバレッジ測定ツールおよびその仕様 | `.gcda`、`.gcno` |
| `lcov` | linux-test-projectが提供するgcovのカバレッジ結果をHTMLなどでグラフィカルに表示するツール | `.gcda`、 `.gcno`、`.info` |
| `llvm-profdata` | LLVM独自のカバレッジデータである`.profraw`ファイルを`.profdata`として集約するツール | `.profraw`、`.profdata` |
| `llvm-cov` | LLVMが提供するClangのカバレッジ測定・表示ツール。gcovとLLVM独自の"source based code coverage"の両仕様に対応している | `.profdata` |


## source-basedカバレッジの計測・表示の流れ

カバレッジ計測・表示の全体像は以下の通りである。

1. カバレッジ測定用の"instrument code"を挿入した状態で、測定対象のプログラムをコンパイルする。

    ```sh
    clang++ foo.cpp -o foo \
        -fprofile-instr-generate \
        -fcoverage-mapping \
        -fcoverage-mcdc
    ```

2. 出来上がったプログラムを実行し、`.profraw`という生プロファイルを生成する。

    ```sh
    LLVM_PROFILE_FILE="foo.profraw" ./foo
    ```

3. 生プロファイルを`.profdata`というインデックス付きプロファイルに集約する。

    ```sh
    llvm-profdata merge -sparse foo.profraw -o foo.profdata
    ```

4. インデックス付きプロファイルを表示する。

    ```sh
    llvm-cov show ./foo -instr-profile=foo.profdata \
        -Xdemangler=c++filt \
        -show-mcdc \
        -show-line-counts-or-regions \
        -show-branches=count
    ```

以下、これらの手順について詳細を説明する。

## 測定対象のソースコード

今回は、下記のようなソースコードのカバレッジ計測を行うものとする。`foo`、`bar`、`buz`、`qux`関数の実装は共通であるが次のような違いがある。

- `foo`は、Branch Coverageが100%となるが、MC/DCは100%とならないような実行となっている。

- `bar`は、MC/DCが100%となるような実行を行っている。（MC/DCが100%であれば、Branch Coverageも100%となる）

- `buz`はテンプレート関数となっており、テンプレートパラメータを区別しなければ`bar`と同様の条件分岐網羅となるように実行しているが、実際には`int`と`long`の二つの型で実体化されており、それぞれを区別するとMC/DCカバレッジが100%とならないようになっている。

- `qux`もテンプレート関数となっており、`int`による特殊化はMC/DCが100%となるが、`long`による特殊化はMC/DCが0%となる。

Branch Coverageは各Condition(= leaf-level boolean expression)が`true`と`false`のそれぞれに少なくとも一度判定されているかどうかを測定したカバレッジであり、対してMC/DCは各Conditionが単独の`true`と`false`の違いでDecision(= composed boolean expression)の`true`と`false`を変化させるような実行の組み合わせがあるかどうかを測定したカバレッジであり、後者の方が厳しい。詳細については、以下の文献が参考となる。

https://llvm.org/devmtg/2022-11/slides/TechTalk4-MCDC-EnablingSafetyCriticalCodeCoverage.pdf

:::message
詳しくは別の記事で述べるが、ここでいうBranch CoverageはいわゆるC2カバレッジ(Condition Coverage)よりも厳しい。[NASAの定義](https://shemesh.larc.nasa.gov/fm/papers/Hayhurst-2001-tm210876-MCDC.pdf)によるCondition Coverageでは、短絡評価を考慮せずに各conditionに`true`と`false`の両方が入力されるかどうかだけがカバレッジの判断基準となっている。一方で、上記のBranch Coverageでは各conditionが`true`と`false`の両方に実際に評価されなければならず、C/C++においては短絡評価の存在を考慮にいれる必要が出てくる。
:::
:::message
GCOVのBranch Coverageはコンパイル後の制御フローグラフ上の分岐網羅を指しており、コンパイル時の最適化結果などによってはLLVMのBranch Coverageの結果と異なり得ることに注意したい。
:::

https://github.com/sf624/zenn-docs/blob/da96b6aa9c0e41d623bc416e15f76e7284de20aa/sample_codes/clang-source-based-coverage/foo.hpp

https://github.com/sf624/zenn-docs/blob/da96b6aa9c0e41d623bc416e15f76e7284de20aa/sample_codes/clang-source-based-coverage/foo.cpp

https://github.com/sf624/zenn-docs/blob/da96b6aa9c0e41d623bc416e15f76e7284de20aa/sample_codes/clang-source-based-coverage/bar.hpp

https://github.com/sf624/zenn-docs/blob/da96b6aa9c0e41d623bc416e15f76e7284de20aa/sample_codes/clang-source-based-coverage/bar.cpp

https://github.com/sf624/zenn-docs/blob/da96b6aa9c0e41d623bc416e15f76e7284de20aa/sample_codes/clang-source-based-coverage/buz.hpp

https://github.com/sf624/zenn-docs/blob/da96b6aa9c0e41d623bc416e15f76e7284de20aa/sample_codes/clang-source-based-coverage/qux.hpp

https://github.com/sf624/zenn-docs/blob/da96b6aa9c0e41d623bc416e15f76e7284de20aa/sample_codes/clang-source-based-coverage/main.cpp

カバレッジ結果は、`coverage.sh`で取得できる。詳細については、以下に説明する。

https://github.com/sf624/zenn-docs/blob/da96b6aa9c0e41d623bc416e15f76e7284de20aa/sample_codes/clang-source-based-coverage/coverage.sh

## 1. カバレッジ測定用コンパイル

まず、測定対象のプログラムを、カバレッジ計測用の"instrument code"を挿入した状態でコンパイルする必要がある。これには、`-fprofile-instr-generate -fcoverage-mapping`というオプションを指定すればよい。

```sh
# インストゥルメントコード付きでコンパイル
clang++-20 foo.cpp bar.cpp main.cpp -o main \
    -fprofile-instr-generate \
    -fcoverage-mapping \
    -fcoverage-mcdc
```

:::message
MCDCカバレッジも表示したい場合は、上記のように`-fcoverage-mcdc`を追加する。
:::

## 2. カバレッジ測定

カバレッジコードが挿入されたプログラムを実行すると、`LLVM_PROFILE_FILE`環境変数に設定されたパスに、カバレッジ結果が含まれる生プロファイル（"raw profile"）が生成される。

```sh
# 生プロファイルを生成
LLVM_PROFILE_FILE="main.profraw" ./main
```

この環境変数を設定しなかった場合は、カレントディレクトリに`default.profraw`として生成される。

```sh
# パスを指定しなければ、`./default.profraw`に生成される
./main
```

何度も同じプログラムを実行する場合、生プロファイルが上書きされないように異なるパスを指定しなければならない。たとえば`%p`といったパターンストリングをパス名に挿入するとプロセスIDに置換してくれる。これらの利用可能なパターンの詳細は[こちら](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html#running-the-instrumented-program)を参照。

```sh
# プロセスID付きで生プロファイルを生成
LLVM_PROFILE_FILE="main-%p.profraw" ./main
```

### gcovとの違い

gcovの場合は、オブジェクトファイルごとに`.gcno`と`.gcda`ファイルが生成された。しかし、こちらの`llvm-cov`の"source based code coverage"の場合は、**1プロセスごとに1つ**の`.profraw`が生成される。従って、gcovのようにオブジェクトファイルごとの測定結果を集約する、というようなプロセスは発生しない。（複数回実行や異なる実行形式の実行結果を集約する場合は、後述するように`llvm-profdata merge`による集約が必要となる。）

## 3. カバレッジ集約

生プロファイル（`.profraw`）はそのままではカバレッジ結果を表示できず、インデックス化をする必要がある。これには`llvm-profdata merge`を使用する。インデックス化をしたあとは、生プロファイルは不要であるため削除しても問題ない。

```sh
# 生プロファイルをインデックス化する
llvm-profdata-20 merge -sparse main.profraw -o main.profdata
```

:::message
`-sparse`オプションによって、`.profdata`のサイズを節約することができるため、基本的には指定すればよい。ただし、プロファイル結果を最適化の材料として使うPGO (Profile Guided Optimization)の場合には使用が推奨されない。
:::

`llvm-profdata merge`は、複数の生プロファイルやインデックス付きプロファイルを集約することも可能である。例えば、共有ライブラリのカバレッジを計測するために、異なるバイナリを実行した結果を集約するといった用途で使える。

```sh
# 生プロファイル同士のマージが可能
llvm-profdata merge -sparse foo1.profraw foo2.profraw -o foo3.profdata

# インデックス付きプロファイルのマージも可能
llvm-profdata merge -sparse foo1.profraw foo2.profdata -o foo3.profdata
```

## 4. カバレッジ結果の表示

### CLIによる表示結果

インデックス付きプロファイル(`.profdata`)は、実行形式も渡すことで`llvm-cov show`によって表示することができる。

```sh
llvm-cov-20 show ./main -instr-profile=main.profdata \
    -Xdemangler=c++filt \
    -show-mcdc \
    -show-line-counts-or-regions \
    -show-branches=count
```

:::message
- `-Xdemangler=c++filt`は、C++で関数名やクラス名が難読化（[名前マングル](https://ja.wikipedia.org/wiki/%E5%90%8D%E5%89%8D%E4%BF%AE%E9%A3%BE)）されてしまうものを読みやすい形にする効果がある（デマングルという）。
- `-show-mcdc`は、MC/DCカバレッジ結果を表示するために指定している。
- `-show-line-counts-or-regions`は、行レベルの実行回数と行内の要素の実行回数が異なる場合に後者を`^`で別途表示する効果を持つ。
- `-show-branches=count`は、各条件分岐の実現回数を表示する。

その他、使用可能なオプションについては[こちら](https://llvm.org/docs/CommandGuide/llvm-cov.html#id5)を参照。
:::

実行結果は以下のようになる（見やすいように表示を入れ替えた）。この結果から分かるように、


- 非テンプレート関数`foo`は、Branch Coverageは100%であるものの、MC/DCは100%とならない。
- 非テンプレート関数`bar`は、MC/DCが100%となる。
- テンプレート関数`buz`は、テンプレートパラメータごとのカバレッジ結果が集計されており、それぞれのMC/DCは100%となっていない。
- テンプレート関数`qux`は、`int`による特殊化についてはMC/DCが100%となる。

また、以下のことも分かる。

- 宣言のみが記載されたファイルは、カバレッジ結果には表示されない。
- テンプレート関数の各行実行回数については、各特殊化別の実行回数とそれを合算したものの2種類が表示される。

```sh
/path/to/foo.cpp:
    1|       |#include "foo.hpp"
    2|       |
    3|      3|void foo(bool a, bool b, bool c) {
    4|      3|    if ((a && b) || c) {
                            ^2    ^2
  ------------------
  |  Branch (4:10): [True: 2, False: 1]
  |  Branch (4:15): [True: 1, False: 1]
  |  Branch (4:21): [True: 1, False: 1]
  ------------------
  |---> MC/DC Decision Region (4:9) to (4:22)
  |
  |  Number of Conditions: 3
  |     Condition C1 --> (4:10)
  |     Condition C2 --> (4:15)
  |     Condition C3 --> (4:21)
  |
  |  Executed MC/DC Test Vectors:
  |
  |     C1, C2, C3    Result
  |  1 { F,  -,  F  = F      }
  |  2 { T,  F,  T  = T      }
  |  3 { T,  T,  -  = T      }
  |
  |  C1-Pair: covered: (1,3)
  |  C2-Pair: not covered
  |  C3-Pair: not covered
  |  MC/DC Coverage for Decision: 33.33%
  |
  ------------------
    5|      2|        volatile int i = 0;
    6|      2|    } else {
    7|      1|        volatile int j = 1;
    8|      1|    }
    9|      3|}
```
```sh
/path/to/bar.cpp:
    1|       |#include "bar.hpp"
    2|       |
    3|      4|void bar(bool a, bool b, bool c) {
    4|      4|    if ((a && b) || c) {
                            ^2    ^3
  ------------------
  |  Branch (4:10): [True: 2, False: 2]
  |  Branch (4:15): [True: 1, False: 1]
  |  Branch (4:21): [True: 1, False: 2]
  ------------------
  |---> MC/DC Decision Region (4:9) to (4:22)
  |
  |  Number of Conditions: 3
  |     Condition C1 --> (4:10)
  |     Condition C2 --> (4:15)
  |     Condition C3 --> (4:21)
  |
  |  Executed MC/DC Test Vectors:
  |
  |     C1, C2, C3    Result
  |  1 { F,  -,  F  = F      }
  |  2 { T,  F,  F  = F      }
  |  3 { F,  -,  T  = T      }
  |  4 { T,  T,  -  = T      }
  |
  |  C1-Pair: covered: (1,4)
  |  C2-Pair: covered: (2,4)
  |  C3-Pair: covered: (1,3)
  |  MC/DC Coverage for Decision: 100.00%
  |
  ------------------
    5|      2|        volatile int i = 0;
    6|      2|    } else {
    7|      2|        volatile int j = 1;
    8|      2|    }
    9|      4|}
```
```sh
/path/to/buz.hpp:
    1|       |#ifndef BUZ_HPP_
    2|       |#define BUZ_HPP_
    3|       |
    4|       |template <typename T>
    5|      4|void buz(bool a, bool b, bool c) {
    6|      4|    if ((a && b) || c) {
                            ^2    ^3
  ------------------
  |  Branch (6:10): [True: 1, False: 2]
  |  Branch (6:15): [True: 1, False: 0]
  |  Branch (6:21): [True: 1, False: 1]
  |  Branch (6:10): [True: 1, False: 0]
  |  Branch (6:15): [True: 0, False: 1]
  |  Branch (6:21): [True: 0, False: 1]
  ------------------
  |---> MC/DC Decision Region (6:9) to (6:22)
  |
  |  Number of Conditions: 3
  |     Condition C1 --> (6:10)
  |     Condition C2 --> (6:15)
  |     Condition C3 --> (6:21)
  |
  |  Executed MC/DC Test Vectors:
  |
  |     C1, C2, C3    Result
  |  1 { F,  -,  F  = F      }
  |  2 { F,  -,  T  = T      }
  |  3 { T,  T,  -  = T      }
  |
  |  C1-Pair: covered: (1,3)
  |  C2-Pair: not covered
  |  C3-Pair: covered: (1,2)
  |  MC/DC Coverage for Decision: 66.67%
  |
  |---> MC/DC Decision Region (6:9) to (6:22)
  |
  |  Number of Conditions: 3
  |     Condition C1 --> (6:10)
  |     Condition C2 --> (6:15)
  |     Condition C3 --> (6:21)
  |
  |  Executed MC/DC Test Vectors:
  |
  |     C1, C2, C3    Result
  |  1 { T,  F,  F  = F      }
  |
  |  C1-Pair: not covered
  |  C2-Pair: not covered
  |  C3-Pair: not covered
  |  MC/DC Coverage for Decision: 0.00%
  |
  ------------------
    7|      2|        volatile int i = 0;
    8|      2|    } else {
    9|      2|        volatile int j = 1;
   10|      2|    }
   11|      4|}
  ------------------
  | void buz<int>(bool, bool, bool):
  |    5|      3|void buz(bool a, bool b, bool c) {
  |    6|      3|    if ((a && b) || c) {
  |                            ^1    ^2
  |  ------------------
  |  |  Branch (6:10): [True: 1, False: 2]
  |  |  Branch (6:15): [True: 1, False: 0]
  |  |  Branch (6:21): [True: 1, False: 1]
  |  ------------------
  |  |---> MC/DC Decision Region (6:9) to (6:22)
  |  |
  |  |  Number of Conditions: 3
  |  |     Condition C1 --> (6:10)
  |  |     Condition C2 --> (6:15)
  |  |     Condition C3 --> (6:21)
  |  |
  |  |  Executed MC/DC Test Vectors:
  |  |
  |  |     C1, C2, C3    Result
  |  |  1 { F,  -,  F  = F      }
  |  |  2 { F,  -,  T  = T      }
  |  |  3 { T,  T,  -  = T      }
  |  |
  |  |  C1-Pair: covered: (1,3)
  |  |  C2-Pair: not covered
  |  |  C3-Pair: covered: (1,2)
  |  |  MC/DC Coverage for Decision: 66.67%
  |  |
  |  ------------------
  |    7|      2|        volatile int i = 0;
  |    8|      2|    } else {
  |    9|      1|        volatile int j = 1;
  |   10|      1|    }
  |   11|      3|}
  ------------------
  | void buz<long>(bool, bool, bool):
  |    5|      1|void buz(bool a, bool b, bool c) {
  |    6|      1|    if ((a && b) || c) {
  |  ------------------
  |  |  Branch (6:10): [True: 1, False: 0]
  |  |  Branch (6:15): [True: 0, False: 1]
  |  |  Branch (6:21): [True: 0, False: 1]
  |  ------------------
  |  |---> MC/DC Decision Region (6:9) to (6:22)
  |  |
  |  |  Number of Conditions: 3
  |  |     Condition C1 --> (6:10)
  |  |     Condition C2 --> (6:15)
  |  |     Condition C3 --> (6:21)
  |  |
  |  |  Executed MC/DC Test Vectors:
  |  |
  |  |     C1, C2, C3    Result
  |  |  1 { T,  F,  F  = F      }
  |  |
  |  |  C1-Pair: not covered
  |  |  C2-Pair: not covered
  |  |  C3-Pair: not covered
  |  |  MC/DC Coverage for Decision: 0.00%
  |  |
  |  ------------------
  |    7|      0|        volatile int i = 0;
  |    8|      1|    } else {
  |    9|      1|        volatile int j = 1;
  |   10|      1|    }
  |   11|      1|}
  ------------------
   12|       |
   13|       |#endif // BUZ_HPP_
```
```sh
/path/to/qux.hpp:
    1|       |#ifndef QUX_HPP_
    2|       |#define QUX_HPP_
    3|       |
    4|       |template <typename T>
    5|      5|void qux(bool a, bool b, bool c) {
    6|      5|    if ((a && b) || c) {
                            ^3    ^4
  ------------------
  |  Branch (6:10): [True: 2, False: 2]
  |  Branch (6:15): [True: 1, False: 1]
  |  Branch (6:21): [True: 1, False: 2]
  |  Branch (6:10): [True: 1, False: 0]
  |  Branch (6:15): [True: 0, False: 1]
  |  Branch (6:21): [True: 0, False: 1]
  ------------------
  |---> MC/DC Decision Region (6:9) to (6:22)
  |
  |  Number of Conditions: 3
  |     Condition C1 --> (6:10)
  |     Condition C2 --> (6:15)
  |     Condition C3 --> (6:21)
  |
  |  Executed MC/DC Test Vectors:
  |
  |     C1, C2, C3    Result
  |  1 { F,  -,  F  = F      }
  |  2 { T,  F,  F  = F      }
  |  3 { F,  -,  T  = T      }
  |  4 { T,  T,  -  = T      }
  |
  |  C1-Pair: covered: (1,4)
  |  C2-Pair: covered: (2,4)
  |  C3-Pair: covered: (1,3)
  |  MC/DC Coverage for Decision: 100.00%
  |
  |---> MC/DC Decision Region (6:9) to (6:22)
  |
  |  Number of Conditions: 3
  |     Condition C1 --> (6:10)
  |     Condition C2 --> (6:15)
  |     Condition C3 --> (6:21)
  |
  |  Executed MC/DC Test Vectors:
  |
  |     C1, C2, C3    Result
  |  1 { T,  F,  F  = F      }
  |
  |  C1-Pair: not covered
  |  C2-Pair: not covered
  |  C3-Pair: not covered
  |  MC/DC Coverage for Decision: 0.00%
  |
  ------------------
    7|      2|        volatile int i = 0;
    8|      3|    } else {
    9|      3|        volatile int j = 1;
   10|      3|    }
   11|      5|}
  ------------------
  | void qux<int>(bool, bool, bool):
  |    5|      4|void qux(bool a, bool b, bool c) {
  |    6|      4|    if ((a && b) || c) {
  |                            ^2    ^3
  |  ------------------
  |  |  Branch (6:10): [True: 2, False: 2]
  |  |  Branch (6:15): [True: 1, False: 1]
  |  |  Branch (6:21): [True: 1, False: 2]
  |  ------------------
  |  |---> MC/DC Decision Region (6:9) to (6:22)
  |  |
  |  |  Number of Conditions: 3
  |  |     Condition C1 --> (6:10)
  |  |     Condition C2 --> (6:15)
  |  |     Condition C3 --> (6:21)
  |  |
  |  |  Executed MC/DC Test Vectors:
  |  |
  |  |     C1, C2, C3    Result
  |  |  1 { F,  -,  F  = F      }
  |  |  2 { T,  F,  F  = F      }
  |  |  3 { F,  -,  T  = T      }
  |  |  4 { T,  T,  -  = T      }
  |  |
  |  |  C1-Pair: covered: (1,4)
  |  |  C2-Pair: covered: (2,4)
  |  |  C3-Pair: covered: (1,3)
  |  |  MC/DC Coverage for Decision: 100.00%
  |  |
  |  ------------------
  |    7|      2|        volatile int i = 0;
  |    8|      2|    } else {
  |    9|      2|        volatile int j = 1;
  |   10|      2|    }
  |   11|      4|}
  ------------------
  | void qux<long>(bool, bool, bool):
  |    5|      1|void qux(bool a, bool b, bool c) {
  |    6|      1|    if ((a && b) || c) {
  |  ------------------
  |  |  Branch (6:10): [True: 1, False: 0]
  |  |  Branch (6:15): [True: 0, False: 1]
  |  |  Branch (6:21): [True: 0, False: 1]
  |  ------------------
  |  |---> MC/DC Decision Region (6:9) to (6:22)
  |  |
  |  |  Number of Conditions: 3
  |  |     Condition C1 --> (6:10)
  |  |     Condition C2 --> (6:15)
  |  |     Condition C3 --> (6:21)
  |  |
  |  |  Executed MC/DC Test Vectors:
  |  |
  |  |     C1, C2, C3    Result
  |  |  1 { T,  F,  F  = F      }
  |  |
  |  |  C1-Pair: not covered
  |  |  C2-Pair: not covered
  |  |  C3-Pair: not covered
  |  |  MC/DC Coverage for Decision: 0.00%
  |  |
  |  ------------------
  |    7|      0|        volatile int i = 0;
  |    8|      1|    } else {
  |    9|      1|        volatile int j = 1;
  |   10|      1|    }
  |   11|      1|}
  ------------------
   12|       |
   13|       |#endif // QUX_HPP_
```
```sh
/path/to/main.cpp:
    1|       |#include "foo.hpp"
    2|       |#include "bar.hpp"
    3|       |#include "buz.hpp"
    4|       |#include "qux.hpp"
    5|       |
    6|      1|int main() {
    7|      1|  foo(false, false, false);
    8|      1|  foo(true, true, false);
    9|      1|  foo(true, false, true);
   10|       |
   11|      1|  bar(false, false, false);
   12|      1|  bar(false, false, true);
   13|      1|  bar(true, true, false);
   14|      1|  bar(true, false, false);
   15|       |
   16|      1|  buz<int>(false, false, false);
   17|      1|  buz<int>(false, false, true);
   18|      1|  buz<int>(true, true, false);
   19|      1|  buz<long>(true, false, false);
   20|       |
   21|      1|  qux<int>(false, false, false);
   22|      1|  qux<int>(false, false, true);
   23|      1|  qux<int>(true, true, false);
   24|      1|  qux<int>(true, false, false);
   25|      1|  qux<long>(true, false, false);
   26|       |
   27|      1|  return 0;
   28|      1|}
```

### HTMLによる表示

HTMLページとして表示したい場合は、`-format=html -output-dir=<path/to/dir>`を追加で指定するとよい。

```sh
llvm-cov-20 show ./main -instr-profile=main.profdata \
    -Xdemangler=c++filt \
    -show-mcdc \
    -show-mcdc-summary \
    -show-line-counts-or-regions \
    -show-branches=count \
    -format=html \
    -output-dir=coverage_html
```

:::message
`-show-mcdc-summary`も指定することで、以下のように全体レポートにMC/DCの統計結果も表示できる。
:::

全体のカバレッジレポートとそれぞれのファイルごとの詳細なカバレッジ結果がグラフィカルに閲覧できる。ファイルごとのカバレッジについては、CLIでは`^`で表示されていたregion countがハイライトされており、マウスオーバーすることでカウントを確認できる。

全体のカバレッジレポートを閲覧するときの注意点として、`qux.hpp`のサマリーを見ると分かるように、テンプレート関数についてはカバレッジが大きく取れている方が表示される。**従って、全体のカバレッジレポートでカバレッジが100%であっても、特殊化の全てが100%とは限らない。**

![](/images/clang-source-based-coverage/image_1.png =600x)
*テンプレート関数のカバレッジ結果のサマリーは、大きい方の特殊化が表示される*

![](/images/clang-source-based-coverage/image_2.png =600x)
*ハイライト部分は行の実行回数と異なる要素を指し、マウスオーバーすると数字が表示される*

### CLIによる全体レポートの表示

全体のレポートは、CLIでも`llvm-cov report`を用いて表示することが可能である。HTMLの場合と同様であるが、テンプレート関数についてはカバレッジが大きく取れている方が表示される。

```sh
$ llvm-cov-20 report ./main -instr-profile=main.profdata -Xdemangler=c++filt -show-mcdc-summary
Filename                      Regions    Missed Regions     Cover   Functions  Missed Functions  Executed       Lines      Missed Lines     Cover    Branches   Missed Branches     Cover    MC/DC Conditions    Missed Conditions     Cover
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bar.cpp                             8                 0   100.00%           1                 0   100.00%           7                 0   100.00%           6                 0   100.00%                   3                    0   100.00%
buz.hpp                             8                 0   100.00%           1                 0   100.00%           7                 0   100.00%           6                 1    83.33%                   3                    1    66.67%
foo.cpp                             8                 0   100.00%           1                 0   100.00%           7                 0   100.00%           6                 0   100.00%                   3                    2    33.33%
main.cpp                            1                 0   100.00%           1                 0   100.00%          19                 0   100.00%           0                 0         -                   0                    0         -
qux.hpp                             8                 0   100.00%           1                 0   100.00%           7                 0   100.00%           6                 0   100.00%                   3                    0   100.00%
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL                              33                 0   100.00%           5                 0   100.00%          47                 0   100.00%          24                 1    95.83%                  12                    3    75.00%
```
