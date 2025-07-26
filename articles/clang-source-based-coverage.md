---
title: "Clangのソースベースド(source-based)カバレッジ計測"
emoji: "📔"
type: "tech" # tech: 技術記事 / idea: アイデア
topics: ["c", "cpp", "clang", "llvm", "coverage"]
published: false
---

## この記事を読むと…

- clangのsource basedカバレッジの計測・取得方法が分かります
- MC/DCカバレッジなどの、高精度なカバレッジ計測ができるようになります

## はじめに

C/C++のカバレッジ計測手法についてはgcovが特に有名であるが、LLVMは独自に"source-based"カバレッジと呼ばれる手法を提供している。gcovでは、カバレッジ計測用のコード（instrument）がコンパイルの最終段階で挿入されるため最適化などの影響を受けやすい。一方で、"source-based"カバレッジではその名の通り、ソースコードレベルでinstrumentが挿入されるため[ほぼ最適化の影響を受けない](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html#impact-of-llvm-optimizations-on-coverage-reports)高精度なカバレッジ測定が可能となっている。

特に、LLVM18からは[MC/DC](https://en.wikipedia.org/wiki/Modified_condition/decision_coverage)（修正条件／決定網羅）という種類のカバレッジが計測できるようになり、より精密で実用的なカバレッジ計測が可能となっている。そこでこの記事では、"source-based"カバレッジの基本的な計測方法を説明し、その計測結果を確認する。

今回は、以下のclangの"source based code coverage"の公式レファレンスをもとに解説した。

https://clang.llvm.org/docs/SourceBasedCodeCoverage.html

また、以下の記事も参考にさせていただいた。

https://qiita.com/joule/items/f9de29ceb1d78c5658d8

## LLVMのバージョンについて

現時点（2025年7月）で安定版の最新版であるLLVM 20を使用する。

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

clangには3つのカバレッジ測定手法があり、混同すると良くないので予め説明する。

- gcov: GCCが提供するカバレッジ測定。最もよく知られているカバレッジ測定手法。
- Sanitizer Coverage: 軽量カバレッジ。実行回数などのカウントはせず、到達したかどうかだけ測定する。
- Source based code coverage: LLVMが独自に提供するカバレッジ測定。gcovよりも詳細な測定が可能であり、最適化による影響を受けづらい。

このうち、今回説明するのは最後の"source based"カバレッジである。

特にgcovとの違いには注意を要する。gcc互換なclangもgcov仕様のカバレッジをサポートしているが、"source based"カバレッジとは別物であり、ネットで検索していて最もヒットしやすいのはこちらのカバレッジである。**`--coverage`オプションを指定した場合は、このgcov形式のカバレッジが計測される。**

```sh
# これらはgcov仕様のカバレッジを計測するコンパイル
g++ --coverage foo.cpp
clang++ --coverage foo.cpp
```

対して、今回説明するsorce-basedカバレッジは以下のように**指定するオプションが異なる**。（なお、上記のオプションと組み合わせた場合は、gcovと"source-based"の両方によるカバレッジ計測が実施される。）

```sh
# これは"source based code coverage"仕様のカバレッジを計測するコンパイル
clang++ -fprofile-instr-generate -fcoverage-mapping foo.cpp
```

このあたり、事情がややこしく混同しやすいため、以下によくカバレッジ測定で出てくるコマンド・用語名とその意味を示す。名前が紛らわしいが、**`lcov`は`llvm-cov`とは別物で**、記事の最後に登場するHTMLのレポートは`lcov`の機能ではなく、`llvm-cov show`によるものである。

| 用語 | 役割 | 関連するファイル |
| - | - | - |
| `gcov` | "GCC"が提供するカバレッジ測定ツールおよびその仕様 | `.gcda`、`.gcno` |
| `lcov` | "linux-test-project"が提供するgcovのカバレッジ結果をHTMLなどでグラフィカルに表示するツール | `.gcda`、 `.gcno`、`.info` |
| `llvm-profdata` | "LLVM"独自のカバレッジデータである`.profraw`ファイルを`.profdata`として集約するツール | `.profraw`、`.profdata` |
| `llvm-cov` | "LLVM"が提供するclangのカバレッジ測定・表示ツール。gcovとLLVM独自の"source based code coverage"の両仕様に対応している | `.profdata` |


## source-basedカバレッジの計測・表示の流れ

カバレッジ計測・表示の全体像は以下の通りである。

1. カバレッジ測定用の"instrument code"を挿入した状態で、測定対象のプログラムをコンパイルする。

    ```sh
    clang++ foo.cpp -o foo \
        -fprofile-instr-generate \
        -fcoverage-mapping
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
    llvm-cov show ./foo -instr-profile=foo.profdata
    ```

以下、これらの手順について詳細を説明する。

## 測定対象のソースコード

今回は、下記のようなソースコードのカバレッジ計測を行うものとする。`main.cpp`、`foo.hpp`、`bar.cpp`、`bar.hpp`の4つのファイルから構成されている。

- `bar`は普通の関数であり、MC/DCカバレッジが100%となるように複数回実行している。

- `foo`はテンプレート関数となっており、テンプレートパラメータを区別しなければ`bar`と同様の条件分岐網羅となるように実行しているが、実際には`int`と`long`の二つの型で実体化されていて、それぞれの実体化についてはMC/DCカバレッジが100%とならないようになっている。

https://github.com/sf624/zenn-docs/blob/main/sample_codes/clang-source-based-coverage/main.cpp

https://github.com/sf624/zenn-docs/blob/main/sample_codes/clang-source-based-coverage/bar.hpp

https://github.com/sf624/zenn-docs/blob/main/sample_codes/clang-source-based-coverage/bar.cpp

https://github.com/sf624/zenn-docs/blob/main/sample_codes/clang-source-based-coverage/foo.hpp

カバレッジ結果は、`coverage.sh`で取得できる。詳細について以下に説明する。

https://github.com/sf624/zenn-docs/blob/main/sample_codes/clang-source-based-coverage/coverage.sh

## 1. カバレッジ測定用コンパイル

まず、測定対象のプログラムを、カバレッジ計測用の"instrument code"を挿入した状態でコンパイルする必要がある。これには、`-fcoverage-mapping -fcoverage-mcdc`というオプションを指定すればよい。

```sh
# インストゥルメントコード付きでコンパイル
clang++-20 main.cpp bar.cpp -o main \
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

gcovの場合は、オブジェクトファイルごとに`.gcno`と`.gcda`ファイルが生成された。しかし、こちらの`llvm-cov`の"source based code coverage"の場合は、**1実行ごとに1つ**の`.profraw`が生成される。従って、gcovのようにオブジェクトファイルごとの測定結果を集約する、というようなプロセスは発生しない。（複数回実行や異なる実行形式の実行結果を集約する場合は、後述するように`llvm-profdata merge`による集約が必要となる。）

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

## 4. カバレッジ結果表示

インデックス付きプロファイル(`.profdata`)は、実行形式も渡すことで`llvm-cov show`によって表示することができる。

```sh
llvm-cov-20 show ./main -instr-profile=main.profdata \
    -Xdemangler=c++filt \
    -show-mcdc \
    -show-line-counts-or-regions \
    -show-branches=count
```

:::message
- `-Xdemangler=c++filt`は、C++で関数名やクラス名が難読化されてしまうものを読みやすい形にする効果がある（デマングル）。
- `-show-mcdc`は、MDCDカバレッジ結果を表示するために指定している。
- `-show-line-counts-or-regions`は、行レベルの実行回数と行内の要素の実行回数が異なる場合に後者を`^`で別途表示する効果を持つ。
- `show-branches=count`は、各条件分岐の実現回数を表示する。

その他、使用可能なオプションについては[こちら](https://llvm.org/docs/CommandGuide/llvm-cov.html#id5)を参照。
:::

実行結果は以下のようになる。見て分かるように、

- 非テンプレート関数`bar`は、MC/DCカバレッジが100%となった。
- テンプレート関数`foo`は、テンプレートパラメータごとのカバレッジ結果が集計されており、それぞれのMC/DCカバレッジは100%となっていない。

```sh
/path/to/sample_codes/clang-source-based-coverage/bar.cpp:
    1|       |#include "bar.hpp"
    2|       |
    3|      3|void bar(const int x, const int y) {
    4|      3|  if ((x > 0) && (y > 0)) {
                               ^2
  ------------------
  |  Branch (4:7): [True: 2, False: 1]
  |  Branch (4:18): [True: 1, False: 1]
  ------------------
  |---> MC/DC Decision Region (4:7) to (4:25)
  |
  |  Number of Conditions: 2
  |     Condition C1 --> (4:7)
  |     Condition C2 --> (4:18)
  |
  |  Executed MC/DC Test Vectors:
  |
  |     C1, C2    Result
  |  1 { F,  -  = F      }
  |  2 { T,  F  = F      }
  |  3 { T,  T  = T      }
  |
  |  C1-Pair: covered: (1,3)
  |  C2-Pair: covered: (2,3)
  |  MC/DC Coverage for Decision: 100.00%
  |
  ------------------
    5|      1|    volatile int i = 0;
    6|      2|  } else {
    7|      2|    volatile int i = 1;
    8|      2|  }
    9|      3|}

/path/to/sample_codes/clang-source-based-coverage/foo.hpp:
    1|       |#pragma once
    2|       |
    3|       |template <typename T>
    4|      3|void foo(const T x, const T y) {
    5|      3|  if ((x > 0) && (y > 0)) {
                               ^2
  ------------------
  |  Branch (5:7): [True: 2, False: 0]
  |  Branch (5:18): [True: 1, False: 1]
  |  Branch (5:7): [True: 0, False: 1]
  |  Branch (5:18): [True: 0, False: 0]
  ------------------
  |---> MC/DC Decision Region (5:7) to (5:25)
  |
  |  Number of Conditions: 2
  |     Condition C1 --> (5:7)
  |     Condition C2 --> (5:18)
  |
  |  Executed MC/DC Test Vectors:
  |
  |     C1, C2    Result
  |  1 { T,  F  = F      }
  |  2 { T,  T  = T      }
  |
  |  C1-Pair: not covered
  |  C2-Pair: covered: (1,2)
  |  MC/DC Coverage for Decision: 50.00%
  |
  |---> MC/DC Decision Region (5:7) to (5:25)
  |
  |  Number of Conditions: 2
  |     Condition C1 --> (5:7)
  |     Condition C2 --> (5:18)
  |
  |  Executed MC/DC Test Vectors:
  |
  |     C1, C2    Result
  |  1 { F,  -  = F      }
  |
  |  C1-Pair: not covered
  |  C2-Pair: not covered
  |  MC/DC Coverage for Decision: 0.00%
  |
  ------------------
    6|      1|    volatile int i = 0;
    7|      2|  } else {
    8|      2|    volatile int i = 1;
    9|      2|  }
   10|      3|}
  ------------------
  | void foo<int>(int, int):
  |    4|      2|void foo(const T x, const T y) {
  |    5|      2|  if ((x > 0) && (y > 0)) {
  |  ------------------
  |  |  Branch (5:7): [True: 2, False: 0]
  |  |  Branch (5:18): [True: 1, False: 1]
  |  ------------------
  |  |---> MC/DC Decision Region (5:7) to (5:25)
  |  |
  |  |  Number of Conditions: 2
  |  |     Condition C1 --> (5:7)
  |  |     Condition C2 --> (5:18)
  |  |
  |  |  Executed MC/DC Test Vectors:
  |  |
  |  |     C1, C2    Result
  |  |  1 { T,  F  = F      }
  |  |  2 { T,  T  = T      }
  |  |
  |  |  C1-Pair: not covered
  |  |  C2-Pair: covered: (1,2)
  |  |  MC/DC Coverage for Decision: 50.00%
  |  |
  |  ------------------
  |    6|      1|    volatile int i = 0;
  |    7|      1|  } else {
  |    8|      1|    volatile int i = 1;
  |    9|      1|  }
  |   10|      2|}
  ------------------
  | void foo<long>(long, long):
  |    4|      1|void foo(const T x, const T y) {
  |    5|      1|  if ((x > 0) && (y > 0)) {
  |                               ^0
  |  ------------------
  |  |  Branch (5:7): [True: 0, False: 1]
  |  |  Branch (5:18): [True: 0, False: 0]
  |  ------------------
  |  |---> MC/DC Decision Region (5:7) to (5:25)
  |  |
  |  |  Number of Conditions: 2
  |  |     Condition C1 --> (5:7)
  |  |     Condition C2 --> (5:18)
  |  |
  |  |  Executed MC/DC Test Vectors:
  |  |
  |  |     C1, C2    Result
  |  |  1 { F,  -  = F      }
  |  |
  |  |  C1-Pair: not covered
  |  |  C2-Pair: not covered
  |  |  MC/DC Coverage for Decision: 0.00%
  |  |
  |  ------------------
  |    6|      0|    volatile int i = 0;
  |    7|      1|  } else {
  |    8|      1|    volatile int i = 1;
  |    9|      1|  }
  |   10|      1|}
  ------------------

/path/to/sample_codes/clang-source-based-coverage/main.cpp:
    1|       |#include "foo.hpp"
    2|       |#include "bar.hpp"
    3|       |
    4|      1|int main() {
    5|      1|  bar(2, 3);
    6|      1|  bar(2, -5);
    7|      1|  bar(-3, 3);
    8|       |
    9|      1|  foo<int>(2, 3);
   10|      1|  foo<int>(2, -5);
   11|      1|  foo<long>(-3, 3);
   12|       |
   13|     11|  for (int i = 0; i < 10; ++i) {
                                        ^10
  ------------------
  |  Branch (13:19): [True: 10, False: 1]
  ------------------
   14|     10|    volatile int j = i;
   15|     10|  }
   16|       |
   17|      1|  return 0;
   18|      1|}
```


HTMLページとして表示したい場合は、`-format=html -output-dir=<path/to/dir>`を追加で指定するとよい。

```sh
llvm-cov-20 show ./main -instr-profile=main.profdata \
    -Xdemangler=c++filt \
    -show-mcdc \
    -show-line-counts-or-regions \
    -show-branches=count \
    -format=html \
    -output-dir=coverage_html
```

全体のカバレッジ結果とそれぞれのファイルごとの詳細なカバレッジ結果がグラフィカルに閲覧できる。

![](/images/clang-source-based-coverage/image_1.png =600x)

![](/images/clang-source-based-coverage/image_2.png =600x)

全体のレポートは、CLIでも`llvm-cov repot`を用いて表示することが可能である。

```sh
$ llvm-cov-20 report ./main -instr-profile=main.profdata -Xdemangler=c++filt -show-mcdc-summary
Filename                      Regions    Missed Regions     Cover   Functions  Missed Functions  Executed       Lines      Missed Lines     Cover    Branches   Missed Branches     Cover    MC/DC Conditions    Missed Conditions     Cover
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
bar.cpp                             6                 0   100.00%           1                 0   100.00%           7                 0   100.00%           4                 0   100.00%                   2                    0   100.00%
foo.hpp                             6                 0   100.00%           1                 0   100.00%           7                 0   100.00%           4                 1    75.00%                   2                    1    50.00%
main.cpp                            4                 0   100.00%           1                 0   100.00%          12                 0   100.00%           2                 0   100.00%                   0                    0         -
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL                              16                 0   100.00%           3                 0   100.00%          26                 0   100.00%          10                 1    90.00%                   4                    1    75.00%
```

:::message
`-show-mcdc-summary`も指定することで、上記のようにレポートにMC/DCの統計結果も表示できる。
:::