---
title: "[ロボティクス] リー群の基礎から最適化まで"
emoji: "💃"
type: "tech" # tech: 技術記事 / idea: アイデア
topics: ["リー群", "最適化", "カルマンフィルタ", "robotics", "python"]
published: false
---

## この記事を読むと…

- 位置姿勢（ポーズ）とは何か
- $\mathrm{SO}(3)$や$\mathrm{SE}(3)$をなぜ位置姿勢（ポーズ）で扱うのか
- 位置姿勢（ポーズ）はなぜ群論と関係があるのか
- 指数写像(Exponential map)・対数写像(Logarithm map)とは何か
- なぜ補間に指数写像・対数写像が出てくるのか
- リー代数とは何か
- 実際に$\mathrm{SE}(2)$で位置姿勢（ポーズ）の補間を計算する方法
- リー群が絡む関数の微分はどう考えるべきか
- $\oplus$や$\ominus$とは何なのか
- $\mathrm{SE}(3)$での点群マッチング最適化を構成する方法
- $\mathrm{SE}(2)$での自己位置推定をカルマンフィルタで構成する方法

これらの事柄について理解できます。

## はじめに

移動体（カメラ、ロボット、ドローン、自動車、航空機、etc.）の位置姿勢を表現し、合成や補間、更には最適化や確率推定をすることは一つの重要な技術である。しかしながら、この分野をいざ調べてみようとすると、やれリー群・リー代数がなんだとか、$\mathrm{SE}(3)$や$\mathfrak{se}(3)$がなんだとか、指数写像(Expmap)・対数写像(Logmap)による補間がどうだとか、などなど見慣れない数学的用語が当たり前のごとく登場する。いざ理解しようと試みても、Wikipediaの記事[^groupwiki][^liewiki][^liealgwiki][^euclisiangroupwiki]は難解だし、厳密な定義から始まる数学の難しそうな教科書や資料がヒットするなどして、工学初学者がこの分野に入っていく上での心理的ハードルは高い。かくいう筆者も、大学生の頃このあたりの文献にぶち当たって、専門用語のオンパレードに圧倒され挫折を覚えた経験がある。更に、このあたりを詳説した日本語の文献は未だにそこまで多くないように見受けられる。

[^groupwiki]: https://ja.wikipedia.org/wiki/%E7%BE%A4_(%E6%95%B0%E5%AD%A6)
[^liewiki]: https://ja.wikipedia.org/wiki/%E3%83%AA%E3%83%BC%E7%BE%A4
[^liealgwiki]: https://ja.wikipedia.org/wiki/%E3%83%AA%E3%83%BC%E4%BB%A3%E6%95%B0
[^euclisiangroupwiki]: https://ja.wikipedia.org/wiki/%E3%83%A6%E3%83%BC%E3%82%AF%E3%83%AA%E3%83%83%E3%83%89%E3%81%AE%E9%81%8B%E5%8B%95%E7%BE%A4

この記事では、そもそも位置姿勢（ポーズ）とは一体何なのかという基礎的だが重要なメンタルモデルをまず構築し、それを表現する数学的な道具の自然な帰結として、$\mathrm{SE}(3)$といった（リー）群にたどり着くことを説明することで、位置姿勢（ポーズ）とこれらの分野とのつながりを理解できるようにつとめる。更に発展的な内容として、リー群が絡む微分の取り扱いを学んだうえで、ガウス・ニュートン法によるポーズ最適化や拡張カルマンフィルタによるポーズ推定への適用方法を述べる。

工学的応用という見地から、直観的な理解を重視して記載するため数学上の厳密な論証は割愛する[^excuse]。前提知識としては、大学初年度の解析学及び線形代数に関する基礎的な知識程度である。

[^excuse]: そもそも私があまり数学に明るくないのでこうなる

## 読み進め方ガイド

元々はより短い記事とするつもりだったが、色々と説明を加えていくうちにかなりのボリュームとなってしまった。最終的には、リー群のテクニックを用いた最適化演算などについて説明をするが、この分野に全く新しい方からすると新しい概念がどんどん登場して圧倒されて終わるだけになると思うので、以下に示すような章で一旦休憩してそこまでの概念を十分理解してから、次に進むようにして頂ければと良いと思われる。

1. そもそもポーズとは何かだったり、ポーズを座標変換行列として表現することに馴染みのない読者

    『群とは何か』までを一旦のゴールとし、リー群を用いたポーズの表現方法に慣れてもらう。

2. ポーズを座標変換行列として表現することは知っていて、その対数写像（リー代数）とは何か、それを通してどのように補間処理が実現できるのかを知りたい読者

    『SE(2)の対数・指数写像による補間』までを一旦のゴールとし、対数写像を通した補間方法について理解してもらう。

3. リー代数とは何かを知っていて、最適化・確率推定などへの応用方法について知りたい読者

    『リー群の微分』から読み始めて頂ければよいと思われる。ただし内容がいささか発展的であり、非線形数理最適化ないしは非線形確率推定に関する経験があるとよい。

## 表記方法

下記のように、記号の小文字・大文字、細字・太字を区別して表記する。混乱を招かないと考えられる場合には、文脈によっては次元を表現する添え字は記載しないことがある。

- $a \in \mathbb{R}$：単一の実数
- $\bm{a} \in \mathbb{R}^n$：実数列ベクトル
- $A \in M(N, \mathbb{R})$：N次の実正方行列
    - $I_N$：N次の単位行列
    - $R_N$：行列式が1でN次の直交行列（＝回転行列）
    - 特にポーズとしての意味を持つリー群の要素については、"Transform"を意味する$T$を用いる場合がある。

また、下記はリー群・リー代数特有の記号や表現方法であり、後に説明されるため読む必要はないが、ここでもまとめて表記法を予め示す。

- $\mathrm{G}$：リー群を表すときは、このようにローマン体を用いる。
- $\mathcal{X}$：リー群の要素を表すときは、場合によってはこのように筆記体を用いる。
  - 前述のように、それが行列であれば行列であることを強調するために$T$を用いることもある
- $\mathfrak{g}$：リー代数を表すときは、このようにフラクトゥール体を用いる。
- $\bm{\xi}$：実数列ベクトルの中でも、リー代数のベクトル表示に対応するものは、このようにギリシャ文字を用いる。
- $\cdot^\vee$：vee演算子。リー代数の要素Aを同型の列ベクトル表示に変換する演算子
- $[\cdot]_\wedge$：wedge演算子。リー代数の列ベクトル表示を元のリー代数に戻す演算子で、vee演算子の逆写像にあたる。
    - 文献によって、$\,\cdot^\wedge\,$、$\,\hat{\cdot}\,$、$[\cdot]_\times$などと表記する場合もある。
- $\exp(\cdot)$：指数写像。特にリー群の文脈においては、リー代数を対応するリー群に変換する。
- $\log(\cdot)$：対数写像。特にリー群の文脈においては、リー群を対応するリー代数に変換する。
- $\operatorname{Exp}(\cdot)$：拡張された指数写像。リー代数のベクトル表示を直接、リー群に変換するものとして定義する。つまり、$\operatorname{Exp}(\cdot) \triangleq \exp([\cdot]_\wedge)$である。元の指数写像と区別するために、大文字のEを用いる。
- $\operatorname{Log}(\cdot)$：拡張された対数写像。リー群を直接、リー代数のベクトル表示に変換するものとして定義する。つまり、$\operatorname{Log}(\cdot) \triangleq (\log(\cdot))^\vee$である。元の指数写像と区別するために、大文字のLを用いる。
- $\oplus$：リー群とリー代数のベクトル表示との和を表す二項演算子であり、後者については対応するリー代数に変換したものを前者と合成するものとする。つまり、$X \oplus y\triangleq X (\operatorname{Exp} y), y \oplus X\triangleq (\operatorname{Exp} y) X$である。
    - 文献によっては、これを$\boxplus$と表記するものもある。
- $\ominus$：2つのリー群の差を、対応するリー代数のベクトル表示として表す二項演算子であり、$\mathcal{Y} \ominus \mathcal{X} \triangleq \operatorname{Log}(\mathcal{X}^{-1} \mathcal{Y})$である。
    - 文献によっては、これを$\boxminus$と表記するものもある。
    - $\operatorname{Log}(\mathcal{Y}\mathcal{X}^{-1} )$としても定義されうるが、今回の用途に当たってはローカル座標系でのリー代数表現が得られる上記のルールに従うものとする。
- $\mathrm{Ad}_T$：リー群の要素$T$の随伴行列。すなわち、$\zeta \oplus T = T \oplus \xi$であるときに、$\zeta = \mathrm{Ad}_T \xi$を満たすような行列。

## 位置姿勢（ポーズ）の表現方法

どのような数学的表現を以て位置・姿勢と我々は呼ぶのか、基礎的な部分であるが非常に重要な部分であるのでここから始める。

簡単な例として、下図のような2次元平面上でのロボットの運動を考える。静止した座標系をグローバル座標(Global frame)、ロボットに固定された動的な座標系をローカル座標(Local frame)と呼ぶことにする[^global]。

[^global]: グローバル座標の例としては、１．地図上に定義された座標（マップ座標）、２．地球に固定された座標（例えば[ECEF][ECEF]座標）、３．地球に固定されない更に純粋な慣性系に近い座標（例えば[ECI][ECI]座標）、などがあり「静止」しているという定義は文脈によって異なる。

[ECEF]: https://en.wikipedia.org/wiki/Earth-centered,_Earth-fixed_coordinate_system
[ECI]: https://en.wikipedia.org/wiki/Earth-centered_inertial

![](/images/lie-group-for-robotics/image1.png =400x)
*このロボットの位置と姿勢はどのように表現すると良いだろうか*

ローカル座標がグローバル座標と一致していたとして、そこからロボットがx-y座標で$\begin{bmatrix}a\\b\end{bmatrix}$移動して、その後に$\theta$だけ反時計回りに回転したと仮定する。このロボットの（元の位置から見たときの）位置・姿勢を何かしらの方法で表現したい。なお、位置・姿勢（＝回転）はあわせてポーズ(pose)という用語で表現される。この言葉が一般的によく登場することから、以後は「ポーズ」を用いて記述する。

### 素朴なポーズ表現

まず、素朴な着想としては登場した三つの数値をシンプルに並べて、

$$
\bm{p} = \begin{bmatrix} a \\ b \\ \theta \end{bmatrix}
$$

のようにポーズを表現するというのが考えられる。

さて、この表現はロボットの運動をそのまま表したもので直観的であるものの応用が利きにくい。

たとえば、ここから更に下図のように**相対的に**ロボットが$\Delta \bm{p} = \begin{bmatrix} \Delta a \\ \Delta b \\ \Delta \theta \end{bmatrix}$に相当する運動をしたと仮定する。この場合、結果としての（グローバル座標から見た）ポーズはどうなるだろうか。

![](/images/lie-group-for-robotics/image2.png =500x)
*ロボットがold local frameから相対的に見て、(Δa、Δb、ΔΘ）に相当する運動をした*

答えは、

$$
\bm{p}_{new} = \begin{bmatrix}a_{new}\\ b_{new}\\ \theta_{new}\end{bmatrix} = \begin{bmatrix} a + \Delta a\cos\theta - \Delta b \sin\theta \\ b + \Delta a \sin\theta + \Delta b \cos\theta \\ \theta + \Delta \theta \end{bmatrix}
$$

となる。当然ながら、$\bm{p}_{new} = \bm{p} + \Delta \bm{p}$のような単純なベクトル和の関係にはなく、これまで慣れ親しんできた演算体系でもない。更に、二次元の場合は回転角については和を取るだけで良かったが、三次元の場合はそんなことも成り立たずより状況が複雑化してくる。

我々は上記で行ったようなポーズの合成演算の他に、ポーズの逆ポーズや、ポーズのべき乗（例えば$\bm{p}$の半分の移動に相当する$\bm{p}^{1/2}$の演算）などの概念を導入して柔軟なポーズの演算を実施したい。従って、このようなロボットの内的な運動から着想を得たポーズの表現方法には限界がある。

### 座標変換として見たポーズ表現

ここで発想を逆にする。任意の**静止している点**のグローバル座標系での座標を$\bm{x}_{global} \in \mathbb{R}^2$とし、移動後のロボットから見た**同じ点**のローカル座標系での座標を$\bm{x}_{local} \in \mathbb{R}^2$と置く。この両者にはどのような等式が成り立つだろうか。


![](/images/lie-group-for-robotics/image3.png =500x)
*図ではx_local, y_localと成分表記しているが、これをまとめて本文では$\bm{x}_{local}$としていることに注意*

これは、よく慣れ親しんだ内容である。先ほどの三つのポーズを表すパラメータ$(a, b, \theta)$から回転行列$R_2(\theta)=\begin{bmatrix}\cos\theta&-\sin\theta\\\sin\theta&\cos\theta\end{bmatrix}$と平行移動ベクトル$\,t=\begin{bmatrix}a\\b\end{bmatrix}$を用意すると、以下の等式が成立する。

$$
\bm{x}_{global} =R \, \bm{x}_{local} + \bm{t}
$$

$R$や$t$が右辺（$\bm{x}_{local}$側）に来ていることに注意してほしい。このことから、移動体のポーズというのは**ローカル座標系からグローバル座標系への座標変換**と等価とみることができる。

ところで、まだ$(R, t)$の2つの数値を別個に取り扱わなければならない。先ほどと同様に、姿勢$(R, t)$から**相対的に**$(\Delta R, \Delta t)$に相当する移動をしたあとのポーズはどのようになるか。

$$
\bm{x}_{global} = R(\Delta R \bm{x}_{local} + \Delta \bm{t}) + \bm{t} = (R \Delta R) \bm{x}_{local} + (R \Delta \bm{t} + \bm{t})
$$

となることから、新しいポーズは$(R \Delta R, R \Delta t + t)$ということになる。先ほどの素朴な表現を用いたときよりはまだ見通しが良くなったが、よりシンプルに扱えないだろうか。

実は先ほどの等式を変形すると、

$$
\begin{bmatrix}\bm{x}_{global}\\ 1 \end{bmatrix} = \begin{bmatrix} R & \bm{t}\\ \bm{0} & 1 \end{bmatrix} \begin{bmatrix} \bm{x}_{local} \\ 1 \end{bmatrix}
$$

が成立することが分かる。$\begin{bmatrix} R & \bm{t}\\ \bm{0} & 1 \end{bmatrix}$はブロック分割による表現を使っており、成分表示すれば$\begin{bmatrix} \cos\theta & -sin\theta & a \\ \sin\theta & \cos\theta & b \\ 0 & 0 & 1 \end{bmatrix}$である。

**この行列$\begin{bmatrix} R & \bm{t}\\ 0 & 1 \end{bmatrix}$をロボットのポーズの表現である**として、$T$と置くことにしよう。また$\bm{x}_{global}$と$\bm{x}_{local}$には1の成分が下に追加されたが、これは同次座標(homogenerous coordinate)と呼ばれるものである。$T$は、ローカル同次座標からグローバル同次座標への座標変換を表現する行列ということになる。

この表現は非常に応用が利く。なぜなら、行列そのものやその演算は我々が良く慣れ親しんだものであり、行列について成立した様々な性質をそのまま応用可能だからである。

たとえば、ポーズ$T_1$から**相対的に**ポーズ$T_2$に相当する運動をしたあとのポーズ$T_3$はどのようになるだろうか。

静止した点に対して、ポーズ$T_1$から見たときのローカル座標を$\bm{x}_{local:1}$、ポーズ$T_3$から見たときのローカル座標を$\bm{x}_{local:3}$としよう。このとき、$T_2$は$\bm{x}_{local:1}$をグローバル座標、$\bm{x}_{local:3}$をローカル座標として見たときの座標変換であるということになる。

$$
\bm{x}_{local:1} = T_2 \bm{x}_{local:3}
$$

従って、

$$
\bm{x}_{global} = T_1 \bm{x}_{local:1} = T_1 T_2 \bm{x}_{local:3} = T_3 \bm{x}_{local:3}
$$

が成立しなければならないので、$T_3 = T_1 T_2$である。ポーズの合成は、行列積という演算と等価であると見なすことができる。

次に、ポーズが$T$であるとき**逆に**ローカル座標から見たグローバル座標のポーズ$T_{inverse}$はどうなるか？

$$
T_{inverse} \bm{x}_{global} = \bm{x}_{local} = T_1^{-1} \bm{x}_{global}
$$

が成立しなければならないので、$T_{inverse} = T^{-1}$である。ポーズの逆演算は、逆行列の演算と等価であると見なすことができる。

さらに、ポーズが$T_1$に至るまでに、同じポーズ$T_2$を2回実施したとすると、$T_2$はどのようにして求められるだろうか。

$$
\bm{x}_{global} = T_1 \bm{x}_{local} = T_2 T_2 \bm{x}_{local}
$$

であるので、$T_1 = T_2^2$であり、従って$T_2 = T_1^{1/2}$である。行列のべき乗は、一般には例えばジョルダン標準形などによって求めることができる[^expmap]。このようにポーズの半分のポーズといった操作は、行列のべき乗の演算であると見なすことができる。

[^expmap]: ただし、リー群の場合はリー代数を通した指数写像・対数写像によって求めることが一般的である。

以上のように、これまでの行列に関する知識体系をそのままポーズの様々な演算に応用可能である。

この$T$の集合は、$\mathrm{SE}(2)$ (SE: Special Euclidian group)と呼ばれるものであり、ユークリッド群(Euclidian group)と呼ばれるものの一つである。また、回転変換のみを実施する$R$単品の集合は$\mathrm{SO}(2)$ (SO: Special orthogonal group)、平行移動変換のみを実施する$t$単品の集合は$\mathrm{T}(2)$ (T: Translation group)と呼ばれるものであり$\mathbb{R}^2$と等価である。

実は、この話は三次元でも全く同じであり、三次元の回転を表す回転行列$R$と平行移動ベクトル$t$を用意すれば、同じ規則が成り立つ。以下に、表でまとめる。

| 群の名称| 群の要素 | 合成演算 | 座標変換 | ポーズとしての意味 |
|-|-|-|-|-|
| $\mathrm{T}(2)$[^translation] | $\bm{t} \in \mathbb{R^2}$ | $\bm{t}_3 = \bm{t}_1 + \bm{t}_2$ | $\bm{x}_{global} = \bm{t} + \bm{x}_{local}$ | 2次元の平行移動 |
| $\mathrm{SO}(2)$ | $T = R_2$ | $T_3 = T_1 T_2$ | $\bm{x}_{global} = T \bm{x}_{local}$ | 2次元の回転 |
| $\mathrm{SE}(2)$ | $T = \begin{bmatrix} R_2 & \bm{t}_2\\ \bm{0} & 1 \end{bmatrix}$  | $T_3 = T_1 T_2$ | $\begin{bmatrix}\bm{x}_{global}\\1\end{bmatrix} = T \begin{bmatrix}\bm{x}_{local}\\1\end{bmatrix}$ | 2次元の剛体運動[^goutai] |
| $\mathrm{T}(3)$[^translation] | $\bm{t} \in \mathbb{R^3}$ | $\bm{t}_3 = \bm{t}_1 + \bm{t}_2$ | $\bm{x}_{global} = \bm{t} + \bm{x}_{local}$ | 3次元の平行移動 |
| $\mathrm{SO}(3)$ | $T = R_3$ | $T_3 = T_1 T_2$ | $\bm{x}_{global} = T \bm{x}_{local}$ | 3次元の回転 |
| $\mathrm{SE}(3)$ | $T = \begin{bmatrix} R_3 & \bm{t}_3 \\ \bm{0} & 1 \end{bmatrix}$  | $T_3 = T_1 T_2$ | $\begin{bmatrix}\bm{x}_{global}\\1\end{bmatrix} = T \begin{bmatrix}\bm{x}_{local}\\1\end{bmatrix}$ | 3次元の剛体運動[^goutai] |

[^goutai]: 大きさや形状が変わらない物体の並進＋回転運動のこと
[^translation]: $\mathrm{T}(2)$や$\mathrm{T}(3)$を同次座標の変換行列として定義する文献もある

### 補足：回転行列の直交性について

二次元の回転も、三次元の回転もそれぞれ2次正方行列、3次正方行列として存在する。任意の回転について、その存在を証明することは他の文献に譲るが、それが直行行列（逆行列が転置行列であるような行列）であることは応用上重要なことなので補足しておく。

回転は原点からの距離を変えない変換である。従って、回転行列$R$がベクトル$\bm{x}$を回転した場合、そのノルムは変化しない。従って、

$$
\bm{x}^T \bm{x} = (R\bm{x})^T (R\bm{x}) = \bm{x}^T R^T R \bm{x}
$$

であり、これが任意の$x$に対して成立するのだから、$R^T R = I$である。よって、$R^{-1} = R^T$でなければならない。

このように、回転行列は逆行列の計算をただの転置として計算できるため、数値計算上、効率的かつ失敗の可能性がない[^inverse]。

[^inverse]: 一般には逆行列を持たない行列がある

これにより、$T \in  \mathrm{SE}(2)$や$T \in \mathrm{SE}(3)$の逆行列も以下のように簡単に求まる。

$$
T^{-1} =
\begin{bmatrix}
R & \bm{t} \\
\bm{0} & 1
\end{bmatrix}^{-1} =
\begin{bmatrix}
R^T & -R^T \bm{t} \\
\bm{0} & 1
\end{bmatrix}
$$

## 群とは何か

$\mathrm{SO}(2)$や$\mathrm{SE}(2)$などは「群(Group)」であるのだが、群とは何だろうか。先ほどの、$\mathrm{SE}(2)$を例にして説明する。（行列積を群の演算として用いる$\mathrm{SO}(2)$、$\mathrm{SO}(3)$、$\mathrm{SE}(3)$でも全く話は同じである）

$\mathrm{SE}(2)$には、以下のような性質がある。

１．任意の2つのポーズに対して、ポーズの合成が行列積として演算され、その結果もポーズである。

$$
\forall T_1, T_2 \in \mathrm{SE}(2),\, T_1 T_2 \in \mathrm{SE}(2)
$$

２．3つ以上のポーズを合成することもできる。この場合、一般に行列積は結合法則を満たすことから、ポーズの合成も結合法則を満たす。

$$
\forall T_1, T_2, T_3 \in \mathrm{SE}(2),\, T_1 (T_2 T_3) = (T_1 T_2) T_3
$$

３．全く動かない、グローバル座標と同一のポーズ（つまり$T=I$ ($I$は単位行列)）もポーズの一つである。これは、他のポーズと合成した場合に同じポーズを返す。

$$
\exists I \in \mathrm{SE}(2),\, \forall T \in \mathrm{SE}(2),\, T I = I T = T 
$$

４．任意のポーズに対して逆演算に相当するポーズが存在し、これは元のポーズに対する逆行列である。これもポーズである。

$$
\forall T \in \mathrm{SE}(2), \exists T^{-1} \in \mathrm{SE}(2), T^{-1} T = T T^{-1} = I
$$

以上のように、ある集合$G$とそれに対して定義される二項演算$\mu$の組$(G,\mu)$が、１．閉性、２．結合法則、３．単位元の存在、４．逆元の存在、の4条件を満たすとき、$(G,\mu)$を「[群](https://ja.wikipedia.org/wiki/%E7%BE%A4_(%E6%95%B0%E5%AD%A6))」と呼ぶ。先ほどの例では、$G$は$\mathrm{SE}(2)$を構成する行列の集合のことであり、$\mu$とは行列積という演算のことである。

また、ポーズは「徐々に」変化させていくことが可能である。つまり、ポーズの微分なる演算が存在することが分かる。このような群を特に「リー群(Lie group)」と呼ぶ。リー群は連続的に変化できる「滑らかな群」で、リー代数との対応を取ることで補間・最適化問題に応用できる。

さらに、$\mathrm{SE}(2)$は同次座標$\bm{x} \in \mathbb{RP}^2$を変換することができ、

０．ポーズは、行列積により座標変換ができる。

$$
\forall T \in \mathrm{SE}(2), \forall \bm{x} \in \mathbb{RP}^2, \, T x \in \mathbb{RP}^2
$$

１．任意の2つのポーズに対して、別々に座標変換をした結果と合成したポーズで座標変換した結果が同じである。

$$
\forall T_1, T_2 \in \mathrm{SE}(2), \forall \bm{x} \in \mathbb{RP}^2, \, (T_1 T_2) \bm{x} = T_1 (T_2 \bm{x})
$$

２．ポーズの単位元（つまり$I$）による座標変換は恒等写像になる。

$$
\forall \bm{x} \in \mathbb{RP}^2, \, I \bm{x} = \bm{x}
$$

という性質を持っている。このような、群$G$による別の集合$X$（上の例では同次座標$\mathbb{RP}^2$のこと）の操作を「群作用（Group action）」といい、群がその集合に「作用（act）」するという。特に、今回の合成された群による作用が１．の順番で作用した結果と同じになるものは「左群作用（Left group action）」と言い、このとき集合$X$は「左 $G$-集合（Left G-set）」と言う。今回であれば、$\mathbb{RP}^2$は左$\mathrm{SE}(2)$-集合である、ということになる。

## 対数写像・指数写像によるポーズの補間計算

例によって、簡単に図示できることから二次元のロボットの運動（ポーズを$\mathrm{SE}(2)$）を扱うが、どのリー群でも同じ議論が成り立つ。次のような問題を考える。

ロボットが時刻$\tau=0$でポーズ$T_1 \in \mathrm{SE}(2)$であった状態から、時刻$\tau=1$でポーズ$T_2 \in \mathrm{SE}(2)$に移動したとしよう。このとき、時刻$0 \le \tau \le 1$におけるポーズ$\mathrm{interp}(T_1, T_2; \tau) \in \mathrm{SE}(2)$を推定したい。つまり、補間計算をしたいということだ。($\mathrm{interp}$はinterpolate(補間)の略)

![](/images/lie-group-for-robotics/interpolate.png =400x)
*青点線は並進成分と回転成分を分離して補間する$\mathrm{T}(2)\times\mathrm{SO}(2)$補間、赤点線は並進成分と回転成分を同時に補間する$\mathrm{SE}(2)$補間を表す。*

こういった問題でまず解法として思いつくのは、位置$\bm{t}$と姿勢$R(\theta)$を分離してそれぞれで線形補間してしまうことである。つまり、

$$
\operatorname{interp}(T_1, T_2; \tau) =
\begin{bmatrix}
R((1-\tau)\theta_1 + \tau\theta_2) & (1-\tau) t_1 + \tau t_2 \\
0 & 1
\end{bmatrix}
$$
$$
\left(
T_1 =
\begin{bmatrix}
R(\theta_1) & \bm{t}_1\\
\bm{0} & 1
\end{bmatrix},\,
T_2 =
\begin{bmatrix}
R(\theta_2) & \bm{t}_2 \\
\bm{0} & 1
\end{bmatrix}
\right)
$$

こういった補間が尤もらしいケースもあるかもしれないが[^drone]、これは上図の青点線に示すように$\mathrm{SE}(2)$としての補間ではない。こういった問題では多くの場合、移動体の進行方向とローカル座標とには大きな関連がある。例えば、自動車であればタイヤの方向に強く進行方向が依存している。

[^drone]: たとえばマルチコプターはヨーイング運動をしながらも、それとは無関係に一定速度ベクトルで移動するというような挙動も可能である。

このことを考慮し、今回は**微小時間あたりのその時点でのローカル座標から見たポーズ変化が一定**であるという制約を課すことにしよう。自動車であれば、ハンドルの角度を一定にしながら一定速度で走行するというような状況[^slip]を想定しているということだ。これは上図の赤点線で示すような軌跡を描き、これが$\mathrm{SE}(2)$としての補間となる。この補間の仕方から想像できるように、$SE(2)$補間では位置の補間結果は円弧を形成する。

[^slip]: その間のスリップの発生量も一定であると仮定する

$SE(2)$補間を考えるにあたり、大きな数$n$を用意して時間$1/n$あたり$\delta T$の微小なポーズ変化を起こしながら、$T_1$から$T_2$に至ったとしよう。

その場合、

$$
T_2 = T_1 \delta T (\delta T)  \cdots (\delta T) = T_1 (\delta T)^n
$$

ということになり、

$$
\operatorname{interp}(T_1, T_2; \tau) = T_1 (\delta T)^{\tau n}
$$

の$n\rightarrow\infty$の極限において$SE(2)$補間を求めることができる。行列の「和の極限」であれば簡単であるが、行列の「積の極限」の極限となると一見すると難しい問題に見えてくる。ただ、以下に示すように実数においては総積を総和の形に変換する方法を学んだことがあるはずだ。

### 1次元の指数・対数写像による乗法的プロセスの補間

一旦、話をシンプルな1次元の実数の場合について考えよう。ある変数$z$が、$\tau=0$で$z=a$で$\tau=1$で$z=b$となったとする。このとき、微小時間$\delta \tau = 1/n$あたり一定の$v \delta \tau$で（加法的に）増加していくと仮定する。このとき、$0 \le \tau \le 1$における$z$の値はいくつになるか？

$$
b = a + \lim_{n \to \infty} \sum_{k=1}^{n} \dfrac{v}{n}
$$

これはすぐに解が求まるが、後述のケースと比較するために敢えて区分求積法を経由すれば、

$$
b - a = \lim_{n \to \infty} \dfrac{1}{n} \sum_{k=1}^{n} v = \int_{0}^{1} v d\tau = v
$$
$$
v = b - a
$$
$$
z = \operatorname{interp}(a, b; \tau) = a + \lim_{n \to \infty} \dfrac{1}{n} \sum_{k=1}^{n\tau} v = a + \int_{0}^{\tau} v d\tau = a + \tau v = a + \tau (b - a)
$$

となり、これは線形補間そのものである。当然だが$\dfrac{dz}{dt} = v$であって、$v$は$z$の速度である。

では、問題を変えて微小時間$\delta \tau = 1/n$あたり**一定の$(1 + \xi \delta \tau)$倍に乗法的に増加していく**と仮定すると答えはどうなるだろうか？

$$
b = a \lim_{n \to \infty} \prod_{k=1}^{n} \left(1 + \dfrac{\xi}{n} \right)
$$

実は、ネイピア数$e$の定義$\lim_{n\to\infty} (1+1/n)^n$を用いればこちらもすぐに答えがでるが、先ほどの加法的なケースと比較するために同じ積分の形に持って行きたい。この場合、両辺に対数を取るのが常套手段である。

$$
\dfrac{b}{a} = \lim_{n \to \infty} \prod_{k=1}^{n} \left(1 + \dfrac{\xi}{n} \right)
$$
$$
\log \dfrac{b}{a} = \lim_{n \to \infty} \sum_{k=1}^{n} \log \left(1 + \dfrac{\xi}{n} \right) = \lim_{n \to \infty} \sum_{k=1}^{n} \dfrac{\xi}{n} = \int_{0}^{1} \xi d\tau = \xi
$$
$$
\xi = \log \dfrac{b}{a}
$$

ただし途中で、$x$が微小なときに成立する近似式$\log (1+x) \approx x$を用いた。

![](/images/lie-group-for-robotics/log-space.png =500x)
*乗法的プロセスは、対数空間を考えると加法的プロセスと見なすことができる*

このように乗法的なプロセスにおいては、対数写像を取った**対数空間において加法的なプロセスと見ることができる**。従って、対数空間における差にあたる$\xi$に線形補間を施した結果に指数写像を取り、それに初期値$a$を掛けることで$z$の補間が得られる。

$$
z(\tau) = \operatorname{interp}(a, b; \tau) = a \exp \left(\tau \xi \right) = a \exp \left(\tau \log \dfrac{b}{a}\right)
$$

ここで$\xi$について見ると、これは対数空間における速度に対応して$\dfrac{d\log z}{d\tau}=\xi$である。では、元の指数空間における速度は何になるかと言えば、$\dfrac{dz}{d\tau} = a \xi \exp(\xi \tau) = z \xi$であり、したがって$z^{-1} \dfrac{dz}{d\tau} = \xi$となる。つまり、対数空間における速度$\xi$は指数空間においては実際の速度の自分自身の逆数を掛けた、相対的な速度に対応することが分かる。特に、$z=1$のときは指数空間での速度と一致する。このことは、次の章でリー群の対数写像を理解する上で重要となる。

### リー群の指数・対数写像による補間

さて、元の$\mathrm{SE}(2)$の場合の補間に話を戻そう。これは乗法的なプロセスであったので、対数写像を取ることで相和の形に持って行くことができる。そもそも行列に指数写像や対数写像を取れることの説明が必要であるが、まずは仮にできると仮定して先に補間式の導出を行おう。

$$
T_2 = T_1 (\delta T)^n
$$

まず、$\delta T$を定数と微小項に分解しよう。全く動かないポーズは単位行列$I$であったので、$\delta T$は$I$に微小な行列を足したものと考えられる。また、その微小項は微小時間$\delta \tau = 1 / N$に比例するものであるとして、その係数を$[\bm{\xi}]_\wedge$と置く[^approx]。この$[\bm{\xi}]_\wedge$は先ほどの一次元の場合と異なって行列である。

[^approx]: 厳密性がなくかなり端折ったが、数学の議論ではないのでお許しいただきたい

$$
\delta T = I + [\bm{\xi}]_\wedge \delta \tau
$$

これより、

$$
T_2 = T_1 \lim_{n \to \infty} \prod_{k=1}^{n} \left(I + \dfrac{1}{n} [\bm{\xi}]_\wedge \right)
$$

であり、左から$T_1^{-1}$を掛けてから、後は先ほどの1次元の場合の対数・指数写像による補間変換と同様の施すことで、

$$
\log (T_1^{-1} T_2) = \lim_{n \to \infty} \sum_{k=1}^{n} \log \left(I + \dfrac{1}{n} [\bm{\xi}]_\wedge \right) = [\bm{\xi}]_\wedge
$$
$$
[\bm{\xi}]_\wedge = \log (T_1^{-1} T_2)
$$
$$
T(\tau) = \operatorname{interp}(T_1, T_2; \tau) =  T_1 \exp (\tau [\bm{\xi}]_\wedge) = T_1 \exp \left(\tau \log (T_1^{-1} T_2)\right) \\
$$

となり、対数写像と指数写像を用いた補間式が得られる。これは、行列積を群の演算としている$\mathrm{SO}(2), \mathrm{SO}(3), \mathrm{SE}(3)$でも全く事情は同じである。（$\mathrm{T}(2), \mathrm{T}(3)$は加法を演算としており、単に線形補間をすればよい。）

$[\bm{\xi}]_\wedge = \log (T_1^{-1} T_2)$であることから、$[\bm{\xi}]_\wedge$はリー群の対数写像として得られる行列になっており、これを「リー代数」と呼ぶ。つまり**リー群とリー代数は指数写像・対数写像によって互いに相互変換できる関係にある**。また、表記法については$\mathrm{SE}(2)$に対応するリー代数は$\mathfrak{se}(2)$というように、小文字のドイツ文字（フラクトゥール体）を用いる。

また、1次元の場合と同様の議論で、$[\bm{\xi}]_\wedge$はリー群の対数空間における速度ベクトルとして捉えることができ、指数空間（リー群）においては$\dfrac{dT}{d\tau} = T [\bm{\xi}]_\wedge$が成立する。では、この速度変化を**相対的に**ポーズ$T$から見たらどうなるだろうか？これは、左から$T^{-1}$を掛ける操作をすればよいのであった。すると、$T^{-1} \dfrac{dT}{d\tau} = [\bm{\xi}]_\wedge$となり一定となる。つまり、**単位時間あたりの相対変化を表現するリー群に対数写像を取って得られるリー代数は、その瞬間のポーズ$T$から相対的に見た（＝ローカル座標から見た）$T$の速度そのもの**ということができる。

特に$T=I$（ローカル座標=グローバル座標）である場合は、$T$のグローバル座標での速度である。このリー代数の捉え方は非常に重要であり、ぜひ頭に入れておきたい。これらの関係を幾何的に図で表すと、以下のようにリー群$\mathrm{G}$の単位元($I$)における接空間（多様体における接平面）がリー代数$\mathfrak{g}$となっていて、これらが対数・指数写像によって相互変換できる関係にある。

![](/images/lie-group-for-robotics/image4.png =300x)
*例：$\mathrm{SE}(3)$と$\mathfrak{se}(3)$は、どちらも4次の実正方行列全体の空間$M_4(\mathbb{R})$に埋め込まれた多様体（超曲面）として見ることができる。そして、$\mathfrak{se}(3)$は$\mathrm{SE}(3)$の単位元($I$)における接空間を構成する。*

## リー代数の指数写像

さて、先ほどの説明で省略した（正方）行列の指数写像・対数写像とは何かから説明する。実数に対する指数関数は、

$$
\exp x = \sum_{k=0}^{\infty} \dfrac{x^k}{k!} = 1 + x + \dfrac{1}{2!} x^2 + \dfrac{1}{3!} x^3 + \cdots 
$$

とべき乗級数で定義されるのであった。

（正方）行列の指数写像とは、この$x$の代わりに行列$X$を代入したものである。この行列は、任意の実数・複素行列に対して存在する（級数が収束する）。

$$
\exp X \triangleq I + \sum_{k=1}^{\infty} \dfrac{X^k}{k!} = I + X +  \dfrac{1}{2!} X^2 + \dfrac{1}{3!} X^3 + \cdots 
$$

そして行列の対数写像は、この指数写像の逆像として定義する。

$$
\log (\exp X) \triangleq X
$$

指数写像は先ほど述べたように任意の実数・複素行列に対して存在するが、対数写像はそうではない。

以上のことを用いて、実際にリー群とリー代数が実際にリー群とリー代数が指数写像・対数写像の関係にあることを示そう。

### リー代数 so(2)

以後、リー群の対数写像から直接計算するのは難しいため、最初にリー代数の行列を提示し、その指数写像が実際にリー群になることを示すことで、リー群とリー代数の対数・指数関係を示す。

結論から示すと、下記のような2次の[歪対称行列（skew-symmetric matrix）](https://ja.wikipedia.org/wiki/%E4%BA%A4%E4%BB%A3%E8%A1%8C%E5%88%97)[^skew]

$$
[\theta]_\wedge =\begin{bmatrix}0&-\theta\\\theta&0\end{bmatrix}
$$

がリー代数$\mathfrak{so}(2)$の要素であり、wedge演算子$[\cdot]_\wedge$は上記のスカラー$\theta$を右辺の歪対称行列の形に変換するものとして定義する。

[^skew]: 交代行列(alternating matrix)ともいう

これを下記のように分解する。

$$
[\theta]_\wedge = \theta [1]_\wedge = \theta\begin{bmatrix}0&-1\\1&0\end{bmatrix}
$$

このとき、$[1]_\wedge^2 = -I_2$が成立する。この性質と$\sin$と$\cos$のべき級数展開を用いて、実際に指数写像を計算してみると、

$$
\begin{align*}
\exp [\theta]_\wedge
&= \exp (\theta [1]_\wedge)
\\
&= I_2 + \theta [1]_\wedge + \dfrac{\theta^2 [1]_\wedge^2}{2!} + \dfrac{\theta^3 [1]_\wedge^3}{3!} + \cdots
\\
&= \left(1 - \dfrac{\theta^2}{2!} + \dfrac{\theta^4}{4!} \cdots\right) I_2 + \left(\theta - \dfrac{\theta^3}{3!} + \dfrac{\theta^5}{5!}\right) [1]_\wedge
\\
&= \cos\theta I_2+ \sin\theta[1]_\wedge
\\
&= \begin{bmatrix}\cos\theta&-\sin\theta\\\sin\theta&\cos\theta\end{bmatrix}
\\
&= R_2(\theta)
\end{align*}
$$

となり、リー群$\mathrm{SO}(2)$である回転行列$R_2$が得られた。従って、$[\theta]_\wedge$がリー代数$\mathfrak{so}(2)$であることが分かった。

そして、$\theta$はまさに$R(\theta)$が表現する回転変換が単位時間で実現する場合の角速度にあたり、リー代数$\mathfrak{so}(2)$がリー群$\mathrm{SO}(2)$の（ローカル座標から見た）速度ベクトルになっていることが分かる。実際に$SO(2)$を微分してみることでもそのことが確認できる。単位時間で$\theta$回転したとすると、時間を$\tau$と置いて、

$$
T = R(\theta \tau) = \begin{bmatrix}\cos\theta \tau&-\sin\theta \tau\\\sin\theta \tau&\cos\theta \tau\end{bmatrix}
$$

である。これを$\tau$で微分を取れば、

$$
\dfrac{dT}{d\tau} = \theta \begin{bmatrix}-\sin\theta \tau&-\cos\theta \tau\\\cos\theta \tau&-\sin\theta \tau\end{bmatrix}
$$

であり、左から$T^{-1}$を掛けて相対速度に変換すると、

$$
\begin{align*}
T^{-1} \dfrac{dT}{d\tau}
&= \theta 
\begin{bmatrix}\cos\theta \tau&\sin\theta \tau\\-\sin\theta \tau&\cos\theta \tau\end{bmatrix}
\begin{bmatrix}-\sin\theta \tau&-\cos\theta \tau\\\cos\theta \tau&-\sin\theta \tau\end{bmatrix}
\\
&= \theta \begin{bmatrix}0&-1\\1&0\end{bmatrix}
\\
&= [\theta]_\wedge
\end{align*}
$$

となり、たしかにリー代数$\mathfrak{so}(2)$が得られる。

### リー代数 se(2)

結論から示すと、$\mathrm{SE}(2)$の対数写像は

$$
[\bm{\xi}]_\wedge = \begin{bmatrix}[\theta]_\wedge&\bm{\rho}\\0&0\end{bmatrix} \quad \left( [\theta]_\wedge \in \mathfrak{so}(2), \bm{\rho} \in \mathbb{2}^3 \right)
$$

であり、これがリー代数$\mathfrak{se}(2)$の要素である。ただし、$\bm{\xi}\triangleq\begin{bmatrix}\bm{\rho}\\\theta\end{bmatrix}\in\mathbb{R}^3$であり、wedge演算子$[\cdot]_\wedge$はこの3次元ベクトルを右辺の行列に変換するものとして定義する。（右辺の行列の中身の$[\theta]_\wedge$については、$\mathrm{so}(2)$で説明したように歪対称行列の形に変換するものとして解釈する。）

実際にこの指数写像を計算してみると、

$$
[\bm{\xi}]_\wedge^k = \begin{bmatrix}[\theta]_\wedge&\bm{\rho}\\0&0\end{bmatrix}^k =
\begin{bmatrix}[\theta]_\wedge^k&[\theta]_\wedge^{k-1}\bm{\rho}\\0&0\end{bmatrix}
$$

であるから、

$$
\exp [\xi]_\wedge = \begin{bmatrix}\exp[\theta]_\wedge&V(\theta)\bm{\rho}\\0&1\end{bmatrix} \in \mathrm{SE}(2)
$$

を得て、リー群$\mathrm{SE}(2)$となることが確認できる。ただし、

$$
V(\theta) = \dfrac{1}{\theta}\begin{bmatrix}\sin\theta&-(1-\cos\theta)\\1-\cos\theta&\sin\theta\end{bmatrix}
$$

であり[^SE2-left-jacobian]、指数写像を取るときに、リー代数の並進成分($\bm{\rho}$)をリー群の並進成分($\bm{t}$)に変換する係数となっている。数値計算において$\theta$が微小な場合は、三角関数のテイラー展開により例えば$\theta$の1次の項で打ち切って

[^SE2-left-jacobian]: この$V(\theta)$は、本稿では詳細には触れないが、$SO(2)$の左ヤコビアン$J_l(\theta)$と一致することから、これを「左ヤコビアン」と呼ぶ文献もある。ただし、$SE(2)$としての左ヤコビアンは別に定義されることから、混同しないように注意されたい。

$$
V(\theta) \approx \begin{bmatrix}1&-\theta/2\\\theta/2&1\end{bmatrix} \quad (\theta \approx 0)
$$

と近似することで0除算を回避できる。

$\theta$は$\mathrm{SO}(2)$のときと同様に相対的な角速度を意味し、$\bm{\rho}$は相対的な並進速度を意味する。また、$[\bm{\xi}]_\wedge$は$\mathrm{SE}(2)$そのものの（相対的な）時間変化率となる。


### リー代数 so(3)

$\mathfrak{so}(2)$のときと同様であるが、下記の3次の歪対称行列がリー代数$\mathfrak{so}(3)$の要素である。

$$
[\bm{\theta}]_\wedge \triangleq \begin{bmatrix}0&-\theta_z&\theta_y\\\theta_z&0&-\theta_x\\-\theta_y&\theta_x&0\end{bmatrix}
\quad \left( \bm{\theta} = \begin{bmatrix}\theta_x\\\theta_y\\\theta_z\end{bmatrix} \right)
$$

これはクロス積行列とも呼ばれ、$[\bm{\theta}]_\times$と表記する文献も多い。なぜなら、これはベクトル$\bm{\theta}$との外積を表す行列であるからだ。実際、別のベクトル$\bm{p} = (p_x, p_y, p_z)^T$を右から掛けると

$$
[\bm{\theta}]_\times \bm{p} = 
\begin{bmatrix}0&-\theta_z&\theta_y\\\theta_z&0&-\theta_x\\-\theta_y&\theta_x&0\end{bmatrix}
\begin{bmatrix}p_x\\ p_y\\ p_z\end{bmatrix} =
\begin{bmatrix}-\theta_zp_y + \theta_yp_z\\ \theta_zp_x-\theta_xp_y\\ -\theta_yp_x + \theta_xp_y\end{bmatrix} = \bm{\theta} \times \bm{p}
$$

となり、ベクトルの外積（クロス積）が得られる。だから、外積を意味する$\times$を下付きの添え字として使って$[\cdot]_\times$のように表す場合がある。

ここで、$[\bm{\theta}]_\wedge$についてベクトル$\bm{\theta}$のノルムを$\theta\triangleq\|\bm{\theta}\|$として、単位ベクトル$\bm{n}$を用いて、

$$
[\bm{\theta}]_\wedge = \theta [\bm{n}]_\wedge = \theta \begin{bmatrix}0&-n_z&n_y\\ n_z&0&-n_x\\ -n_y&n_x&0\end{bmatrix} \quad ( \|\bm{n}\| = 1)
$$

と分解する。このとき、$[\bm{n}]_\wedge^3 = -[\bm{n}]_\wedge$が成立する。この性質と$\sin$と$\cos$のテイラー展開を活用して、実際に指数写像を計算すると、

$$
\begin{align*}
\exp [\bm{\theta}]_\wedge
&= \exp (\theta [\bm{n}]_\wedge) 
\\
&= I + \theta [\bm{n}]_\wedge + \dfrac{\theta^2 [\bm{n}]_\wedge^2}{2!} + \dfrac{\theta^3 [\bm{n}]_\wedge^3}{3!} + \cdots
\\
&= I + \left(\theta - \dfrac{\theta^3}{3!} + \dfrac{\theta^5}{5!}\right) [\bm{n}]_\wedge + \left(\dfrac{\theta^2}{2!} - \dfrac{\theta^4}{4!} + \dfrac{\theta^6}{6!} \cdots\right) [\bm{n}]_\wedge^2
\\
&= I + \sin\theta[\bm{n}]_\wedge + (1-\cos\theta)[\bm{n}]_\wedge^2
\end{align*}
$$

が成立する。これは「[ロドリゲスの回転公式](https://ja.wikipedia.org/wiki/%E3%83%AD%E3%83%89%E3%83%AA%E3%82%B2%E3%82%B9%E3%81%AE%E5%9B%9E%E8%BB%A2%E5%85%AC%E5%BC%8F)」と一致し、単位ベクトル$\bm{n}$を軸に（右手の法則に従って）角度$\theta$の回転を行う回転行列である。従って、これはリー群$\mathrm{SO}(3)$であるので、$[\bm{\theta}]_\wedge$はリー代数$\mathfrak{so}(3)$であることが示された。$\bm{\theta}$はまさに角速度ベクトルに対応し、$[\bm{\theta}]_\wedge$はリー群$\mathrm{SO}(3)$の（相対的な）時間変化率である。

### リー代数 se(3)

$\mathrm{SE}(2)$のときと同様に、$\mathrm{SE}(3)$の対数写像は

$$
[\bm{\xi}]_\wedge = \begin{bmatrix}[\bm{\theta}]_\wedge&\bm{\rho}\\0&0\end{bmatrix} \quad \left( [\bm{\theta}]_\wedge \in \mathrm{SO}(3), \rho \in \mathbb{R}^3 \right)
$$

であり、これがリー代数$\mathfrak{se}(3)$の要素である。ただし、$\bm{\xi} \triangleq \begin{bmatrix}\bm{\rho}\\\bm{\theta}\end{bmatrix} \in \mathbb{R}^6$であり、wedge演算子$[\cdot]_\wedge$はこの6次元ベクトルを右辺の行列に変換するものとする。

この指数写像は、

$$
\exp [\bm{\xi}]_\wedge = \begin{bmatrix}\exp[\bm{\theta}]_\wedge&V(\bm{\theta})\bm{\rho}\\0&1\end{bmatrix} \in \mathrm{SE}(3)
$$

である。ただし、

$$
V(\bm{\theta}) = I_3 + \dfrac{1-\cos\theta}{\theta^2} [\bm{\theta}]_\wedge + \dfrac{\theta - \sin\theta}{\theta^3} [\bm{\theta}]_\wedge^2
$$

である[^SE3-left-jacobian]。数値計算において$\theta$が微小な場合は、三角関数のテイラー展開により例えば$\theta$の1次の項で打ち切って

[^SE3-left-jacobian]: この$V(\bm{\theta})$は、本稿では詳細には触れないが、$SO(3)$の左ヤコビアン$J_l(\bm{\theta})$と一致することから、これを「左ヤコビアン」と呼ぶ文献もある。ただし、$SE(3)$としての左ヤコビアンは別に定義されることから、混同しないように注意されたい。

$$
V(\theta) \approx I_3 + \dfrac{1}{2}[\bm{\theta}]_\wedge
$$

として0除算を回避できる。

$\mathfrak{se}(2)$のときと同様に、$\bm{\theta}$は（相対的な）角速度ベクトル、$\bm{\rho}$は（相対的な）並進速度ベクトルであり、$[\bm{\xi}]_\wedge$は$\mathrm{SE}(3)$そのものの（相対的な）時間変化率である。



### リー群とリー代数のまとめ

ここまでで分かったリー群とリー代数の具体的な形を以下にまとめる。

$M_N(\mathbb{R})$とは、N次実正方行列を指す。注意してほしいのは、対数・指数写像は乗法（行列積）を演算として採用したリー群（$\mathrm{SO}(2),\mathrm{SE}(2),\mathrm{SO}(3),\mathrm{SE}(3)$）とそのリー代数に対して定義されるのであって、加法を演算として採用したリー群（$\mathrm{T}(2),\mathrm{T}(3)$）では扱わない。

| リー群 | 対応するリー代数 |
| - | - |
| $\mathrm{T}(2) = \mathbb{R}^2$ (ただし群の演算としては加法を用いる) | $\mathfrak{t}(2) = \mathbb{R}^2$ |
| $\mathrm{SO}(2) = \left \{ R = M_2(\mathbb{R}) \| R^{-1} = R^T, \det(R) = 1 \right \}$ | $\mathfrak{so}(2) = \left \{ [\theta]_\wedge = M_2(\mathbb{R}) \| [\theta]_\wedge + [\theta]_\wedge^T = 0 \right \}$ |
| $\mathrm{SE}(2) = \left \{ \begin{bmatrix}R_2&t_2\\0&1\end{bmatrix} \| R_2 \in \mathrm{SO}(2), t \in \mathrm{T}(2) \right \}$ | $\mathfrak{se}(2) = \left \{ \begin{bmatrix}[\theta]_\wedge&\rho\\0&0\end{bmatrix} \| [\theta]_\wedge \in \mathfrak{so}(2), \rho \in \mathrm{T}(2) \right \}$ |
| $\mathrm{T}(3) = \mathbb{R}^3$ (ただし群の演算としては加法を用いる) | $\mathfrak{t}(3) = \mathbb{R}^3$ |
| $\mathrm{SO}(3) = \left \{ R = M_3(\mathbb{R}) \| R^{-1} = R^T, \det(R) = 1 \right \}$ | $\mathfrak{so}(3) = \left \{ [\theta]_\wedge = M_3(\mathbb{R}) \| [\theta]_\wedge + [\theta]_\wedge^T = 0 \right \}$ |
| $\mathrm{SE}(3) = \left \{ \begin{bmatrix}R_3&t_3\\0&1\end{bmatrix} \| R_3 \in \mathrm{SO}(3), t \in \mathrm{T}(3) \right \}$ | $\mathfrak{se}(3) = \left \{ \begin{bmatrix}[\theta]_\wedge&\rho\\0&0\end{bmatrix} \| [\theta]_\wedge \in \mathfrak{so}(3), \rho \in \mathrm{T}(3) \right \}$ |

### SE(2)の対数・指数写像による補間

さて、ようやく道具は揃った。実際に、$\mathrm{SE}(2)$による補間を行ってみよう。

時刻$\tau = 0$で$\begin{bmatrix}x\\y\\\theta\end{bmatrix}=\begin{bmatrix}1\\0\\\pi/2\end{bmatrix}$、時刻$\tau = 1$で$\begin{bmatrix}x\\y\\\theta\end{bmatrix}=\begin{bmatrix}0\\1\\\pi\end{bmatrix}$と動いたときのロボットについて、$0<\tau<1$での$\mathrm{SE}(2)$補間を求めたい。先ほどの説明から想像されるように、この補間結果は単位円上に来ると予想される。

![](/images/lie-group-for-robotics/interpolate2.png =400x)
*$\mathrm{SE}(2)$補間であれば、補間結果は単位円上に来るはずである。*

リー群の補間は以下の式で求められるのであった。

$$
T(\tau) = \operatorname{interp}(T_1, T_2; \tau) = T_1 \exp \left(\tau \log (T_1^{-1} T_2)\right) \\
$$

ここで、

$$
T_1 = \begin{bmatrix}R\left(\dfrac{\pi}{2}\right)&\begin{matrix}1\\0\end{matrix}\\\bm{0}&1\\\end{bmatrix}=\begin{bmatrix}0&-1&1\\1&0&0\\0&0&1\\\end{bmatrix}, T_2 = \begin{bmatrix}R(\pi)&\begin{matrix}0\\1\end{matrix}\\\bm{0}&1\\\end{bmatrix} = \begin{bmatrix}-1&0&0\\0&-1&1\\0&0&1\\\end{bmatrix}
$$

である。

また、対数写像を計算する必要があるが、再掲すると、$\mathfrak{se}(2)$の指数写像として得られる$\mathrm{SE}(2)$は下記のようになるのだった。ただし、回転行列については$\exp[\theta]_\wedge$の代わりに$R(\theta)$を使って表記している。

$$
\begin{align*}
\exp [\bm{\xi}]_\wedge &= \begin{bmatrix}R(\theta)&V(\theta)\bm{\rho}\\0&1\end{bmatrix} \in \mathrm{SE}(2) \quad ([\bm{\xi}]_\wedge \in \mathfrak{se}(2))
\\
V(\theta) &= \dfrac{1}{\theta}\begin{bmatrix}\sin\theta&-(1-\cos\theta)\\1-\cos\theta&\sin\theta\end{bmatrix}
\end{align*}
$$

まず、$T_1^{-1}T_2 = \begin{bmatrix}0&-1&1\\1&0&1\\0&0&1\end{bmatrix}$であり、この対数写像$\log(T_1^{-1}T_2)$を求めたい。$T_1^{-1}T_2$の回転成分は$\theta=\dfrac{\pi}{2}$であり[^atan2]、これより$J\left(\dfrac{\pi}{2}\right)=\dfrac{2}{\pi}\begin{bmatrix}1&-1\\1&1\end{bmatrix}$であるから、$\bm{\rho}=J\left(\dfrac{\pi}{2}\right)^{-1}\begin{bmatrix}1\\1\end{bmatrix}=\begin{bmatrix}\pi/2\\0\end{bmatrix}$であり、$\bm{\xi} = \begin{bmatrix}\theta\\\bm{\rho}\end{bmatrix} = \begin{bmatrix}\pi/2\\\pi/2\\0\\\end{bmatrix}$のように求められる。

[^atan2]: たとえば行列の(2,1)成分をy、(1,1)成分をxとしたときのatan2(y,x)を求めればよい

これにより、

$$
\begin{align*}
\exp(\tau\log(T_1^{-1}T_2))
&=\exp(\tau[\bm{\xi}]_\wedge)
\\
&=\begin{bmatrix}R\left(\dfrac{\pi\tau}{2}\right)&J\left(\dfrac{\pi\tau}{2}\right)\rho\\\bm{0}&1\end{bmatrix}
\\
&= \begin{bmatrix}R\left(\dfrac{\pi\tau}{2}\right)&\begin{matrix}\sin\dfrac{\pi\tau}{2}\\[8pt]1-\cos\dfrac{\pi\tau}{2}\end{matrix}\\\bm{0}&1\end{bmatrix}
\end{align*}
$$

となる。以上より、

$$
\begin{align*}
T(\tau) &= \operatorname{interp}(T_1, T_2; \tau)
\\
&= T_1 \exp \left(\tau \log (T_1^{-1} T_2)\right)
\\
&= \begin{bmatrix}R\left(\dfrac{\pi}{2}\right)&\begin{matrix}1\\0\end{matrix}\\\bm{0}&1\\\end{bmatrix} \begin{bmatrix}R\left(\dfrac{\pi\tau}{2}\right)&\begin{matrix}\sin\dfrac{\pi\tau}{2}\\[8pt]1-\cos\dfrac{\pi\tau}{2}\end{matrix}\\\bm{0}&1\end{bmatrix}
\\
&= \begin{bmatrix}R\left(\dfrac{(1+\tau)\pi}{2}\right)&\begin{matrix}\cos\dfrac{\pi\tau}{2}\\[8pt]\sin\dfrac{\pi\tau}{2}\end{matrix}\\\bm{0}&1\end{bmatrix}
\end{align*}
$$

を得る。これは、まさに単位円上の等速円運動を表す。

### ローカル座標の速度・加速度によるSE(2)の更新

ロボットが、初期ポーズが$T_0$で、その後$\Delta \tau_i (i = 1\sim n)$の時間間隔でローカル座標系で速度$\bm{u}_i = [\bm{v}_i, \omega_i]^T$で走行した場合の最終的なポーズ$T \in \mathrm{SE}(2)$を求めたいとする。これは、非常に簡単であり、

$$
T = T_0 \exp([\bm{u}_1]_\wedge \tau_1) \exp([\bm{u}_2]_\wedge \tau_2) \cdots \exp([\bm{u}_n]_\wedge \tau_n)
$$

として求めることができる。このように、ローカル座標における相対的な速度情報を用いたポーズの更新は単純な指数写像の積として求めることができる。これは、$\mathrm{SE}(3)$であっても同様である。数式の内容は全く変わらないものの、後に導入する$\oplus$演算子を用いれば、

$$
T = T_0 \oplus \bm{u}_1 \tau_1 \oplus \bm{u}_2 \tau_2 \cdots \oplus \bm{u}_n \tau_n
$$

のようにも表記でき、あたかも単純な加法で更新後のポーズを得られるような表現となる。

## リー群の微分

:::message
記号の使用方法などに関しては文献によって多少の違いがあり、ここではJoan Solà氏の『[A micro Lie theory for state estimation in robotics](https://arxiv.org/abs/1812.01537)』の表記方法に倣う。
:::

非線形数理最適化や非線形確率推定の分野では、ベクトル値関数の微分計算が重要になることが多い。

ニュートン法であれば、

$$
\bm{x} = \bm{x}_0 - \left(\dfrac{\partial f}{\partial \bm{x}}(\bm{x}_0)\right)^{-1} f(\bm{x}_0)
$$

のように更新することで、ベクトル値関数$f: \mathbb{R}^n \to \mathbb{R}^n$の停留点を求めることができる。

拡張カルマンフィルタであれば、時間更新式においては

$$
P_{k|k+1} = F_k P_{k-1|k+1} F_k^T + G_k Q_k G_k^T
$$

$$
F_k \triangleq \left.\dfrac{\partial f}{\partial \bm{x}}\right|_{\hat{\bm{x}}_{k-1|k-1}, \bm{u}_k}, G \triangleq \left.\dfrac{\partial f}{\partial \bm{w}}\right|_{\bm{x}_{k-1}, \bm{u}_k}
$$

のようにすることで、状態量$\bm{x}$の誤差共分散行列$P$を更新することができる。

以上のような既存の手法を活用し、リー群であるポーズを最適化ないし確率推定したいケースがある。前者であれば例えば点群マッチングであり、後者であれば例えばオドメトリとセンサによるポーズ推定などである。この章では、リー群が絡む場合の微分をどう扱うべきかについて解説する。

まず、おさらいとして通常のベクトル値関数の微分を振り返る。ベクトル$\bm{x} \in \mathbb{R}^n$を引数に取るベクトル値関数$f: \mathbb{R}^n \to \mathbb{R}^m$では、

$$
J_f \triangleq \dfrac{\partial f(\bm{x})}{\partial \bm{x}} \triangleq \lim_{\delta x \to 0} \dfrac{f(\bm{x}+\delta\bm{x}) - f(\bm{x})}{\delta \bm{x}}
$$

のように微分が定義され、これを$J_f \in \mathbb{R}^{m \times n}$をヤコビ行列（Jacobian matrix）と呼んだのだった。これを用いると、

$$
f(\bm{x} + \Delta \bm{x}) \approx f(\bm{x}) + J_f \Delta \bm{x}
$$

のように、関数$f$の微小変化をヤコビ行列で表現することができる。

では、もし$f$がリー群を引数に取り、別のリー群を出力する関数$f: \mathrm{G} \to \mathrm{H}$であった場合、$f$の微分はどのように捉えるべきだろうか？先ほどのベクトル値関数の微分をそのまま適用すると以下のようになる。

$$
J_f \stackrel{???}{=} \dfrac{\partial f(\mathcal{X})}{\partial \mathcal{X}} \stackrel{???}{=} \lim_{\delta \mathcal{X} \to 0} \dfrac{f(\mathcal{X}+\delta\mathcal{X}) - f(\mathcal{X})}{\delta \mathcal{X}} \quad \text{(ill-defined)}
$$

ただし、微分においては$\mathcal{X}$は例えば各列ベクトルをつなげて単一の列ベクトルと見なして、先ほどと同様の微分を施すものとする。ここで以下のように2つの問題が発生する。

1. $\mathcal{X}+\delta\mathcal{X}$がリー群であるためには$\delta \mathcal{X}$は制約条件を持ったベクトルになる。これは、リー群$\mathrm{G}$が加法に対してベクトル空間を形成しないことによる。

2. $f(\mathcal{X}+\delta\mathcal{X}) - f(\mathcal{X})$の結果もリー群$G$であってほしいが、1.の場合と同様にリー群はベクトル空間でないため、単純な差を取った場合にはそれは保証されない。このままでは、得られたヤコビ行列を用いて$f(\bm{x} + \Delta \bm{x}) \approx f(\bm{x}) + J_f \Delta \bm{x}$といった近似式を得ることができない。

そこで、リー群の微分を考えるにあたっては、リー群の微小変化を記述できる制約条件のないベクトルが欲しい。それが都合が良いことに存在し、それこそがリー代数である。リー代数は前述した形からも分かるように定数倍や和はリー代数となり、ベクトル空間[^vector-space]を形成する。そこで、リー群と微小なリー代数から得られるリー群との行列積を「和」として捉え、またその逆を差として捉えることが可能である。そこで、

[^vector-space]: ここでいうベクトルとは、いわゆる列ベクトルのことではなくリー代数の行列そのものが「[ベクトル](https://ja.wikipedia.org/wiki/%E3%83%99%E3%82%AF%E3%83%88%E3%83%AB%E7%A9%BA%E9%96%93)」としての性質を満たすという意味である。またリー代数のベクトル表示を用いれば、それはまさしく通常の意味でのベクトルである。

$$
\begin{align*}
J_f \triangleq \dfrac{D f(\mathcal{X})}{D \mathcal{X}} &\triangleq \lim_{\delta \bm{\xi} \to \bm{0}} \dfrac{f(\mathcal{X}\oplus\delta\bm{\xi}) \ominus f(\mathcal{X})}{\delta \bm{\xi}}
\\
&\triangleq \dfrac{\partial \operatorname{Log}(f(\mathcal{X})^{-1} f(\mathcal{X} \operatorname{Exp}(\bm{\xi})))}{\partial \bm{\xi}}
\end{align*}
$$

のように$\mathcal{X}$のまわりの微小変化を、制約条件のないリー代数（のベクトル表示）$\bm{\xi}$を用いて表現することで、微分を定義することが可能となる。この結果得られるヤコビ行列の列サイズは、リー代数のベクトル表示の次元数（＝リー群・リー代数の自由度）と同じになり、

$$
f(\mathcal{X} \oplus \Delta \bm{\xi}) \approx f(\mathcal{X}) \oplus J_f \Delta \bm{\xi}
$$

のように、近似式を得ることができる。なお、もはやこの微分は通常の意味での（偏）微分ではないため、（偏）微分を表す$\partial$は用いず$D$と表記して区別している。

ただし、新しく登場した記号$\oplus$と$\ominus$は普通の意味での和や差ではなく、下記のように定義される。

- $\operatorname{Exp}(\cdot)$：拡張された指数写像。リー代数のベクトル表示を直接、リー群に変換するものとして定義する。つまり、$\operatorname{Exp}(\cdot) \triangleq \exp([\cdot]_\wedge)$である。元の指数写像と区別するために、大文字のEを用いる。
- $\operatorname{Log}(\cdot)$：拡張された対数写像。リー群を直接、リー代数のベクトル表示に変換するものとして定義する。つまり、$\operatorname{Log}(\cdot) \triangleq (\log(\cdot))^\vee$である。元の指数写像と区別するために、大文字のLを用いる。
- $\oplus$：リー群とリー代数のベクトル表示との和を表す二項演算子であり、後者については対応するリー代数に変換したものを前者と合成するものとする。つまり、$X \oplus y\triangleq X (\operatorname{Exp} y), y \oplus X\triangleq (\operatorname{Exp} y) X$である。
    - 文献によっては、これを$\boxplus$と表記するものもある。
- $\ominus$：2つのリー群の差を、対応するリー代数のベクトル表示として表す二項演算子であり、$\mathcal{Y} \ominus \mathcal{X} \triangleq \operatorname{Log}(\mathcal{X}^{-1} \mathcal{Y})$である。
    - 文献によっては、これを$\boxminus$と表記するものもある。
    - $\operatorname{Log}(\mathcal{Y}\mathcal{X}^{-1} )$としても定義されうるが、今回の用途に当たってはローカル座標系でのリー代数表現が得られる上記のルールに従うものとする。

さて、今回は一気にリー群からリー群への関数の微分を取り扱ったが、実際には入力だけリー群であったり、出力だけリー群であったりする。その場合は、$\oplus$と$\ominus$は必要な方だけ採用すればよい。つまり、以下のような組み合わせとなる。

1. 関数$f$がベクトルからベクトルへの関数である。$f: \mathbb{R}^n\to\mathbb{R}^m$

    通常のベクトル値関数の微分を用いる。このときヤコビ行列は$m\times n$の行列となる。

$$
J_f \triangleq \dfrac{\partial f(\bm{x})}{\partial \bm{x}} \triangleq \lim_{\delta x \to 0} \dfrac{f(\bm{x}+\delta\bm{x}) - f(\bm{x})}{\delta \bm{x}}
$$

$$
f(\bm{x} + \Delta \bm{x}) \approx f(\bm{x}) + J_f \Delta \bm{x}
$$

2. 関数$f$がベクトルからリー群への関数である。$f: \mathbb{R}^n\to\mathrm{G}$

    通常の和（$+$）とリー群用の差（$\ominus$）を用いる。このときヤコビ行列は$m\times n$の行列となり、$m$はリー群$\mathrm{G}$の自由度である。

$$
J_f \triangleq \dfrac{D f(\bm{x})}{D \bm{x}} \triangleq \lim_{\delta \bm{x} \to \bm{0}} \dfrac{f(\bm{x}+\delta\bm{x}) \ominus f(\bm{x})}{\delta \bm{x}} 
$$

$$
f(\bm{x} + \Delta \bm{x}) \approx f(\bm{x}) \oplus J_f \Delta \bm{x}
$$


3. 関数$f$がリー群からベクトルへの関数である。$f: \mathrm{G}\to\mathbb{R}^n$

    リー群用の和（$\oplus$）と通常の差（$-$）を用いる。このときヤコビ行列は$m\times n$の行列となり、$n$はリー群$\mathrm{G}$の自由度である。

$$
J_f \triangleq \dfrac{D f(\mathcal{X})}{D \mathcal{X}} \triangleq \lim_{\delta \bm{\xi} \to \bm{0}} \dfrac{f(\mathcal{X}\oplus\delta\bm{\xi}) - f(\mathcal{X})}{\delta \bm{\xi}} 
$$

$$
f(\mathcal{X} \oplus \Delta \bm{\xi}) \approx f(\mathcal{X}) + J_f \Delta \bm{\xi}
$$


4. 関数$f$がリー群からリー群への関数である。$f: \mathrm{G}\to\mathrm{H}$

    リー群用の和（$\oplus$）とリー群の差（$\ominus$）を用いる。このときヤコビ行列は$m\times n$の行列となり、$m$はリー群$\mathrm{H}$の自由度、$n$はリー群$\mathrm{G}$の自由度である。

$$
J_f \triangleq \dfrac{D f(\mathcal{X})}{D \mathcal{X}} \triangleq \lim_{\delta \bm{\xi} \to \bm{0}} \dfrac{f(\mathcal{X}\oplus\delta\bm{\xi}) \ominus f(\mathcal{X})}{\delta \bm{\xi}} 
$$

$$
f(\mathcal{X} \oplus \Delta \bm{\xi}) \approx f(\mathcal{X}) \oplus J_f \Delta \bm{\xi}
$$

### SE(3)における3次元点群マッチングによるポーズ推定

:::message
scomup氏の『[ロボット技術者向け リー群の速習(4) リー群を用いた最適化](https://qiita.com/scomup/items/a9c09d57101583c58619)』の内容をベースに解説する。
:::

異なる位置で観測された点群$\{\bm{a}_i\}$と$\{\bm{b}_i\}$が与えられ、添え字$i$が同一のものが対応していると仮定するとき、$\{\bm{a}_i\}$を最もよく$\{\bm{b}_i\}$に近づけられるような座標変換$T$を考えたい。コスト関数として例えば以下のように座標変換後のそれぞれの対応する点の距離の二乗和を定義し、これを最小化するような$T_{opt}$が最善の座標変換だとし、これを求めることにする。

$$
\begin{align}
T_{opt} \triangleq \sum_{i=1}^n \| \bm{f}_i(T) \|^2 &\triangleq \argmin_T \sum_{i=1}^n \| T \cdot \bm{a}_i - \bm{b}_i \|^2
\\
&= \sum_{i=1}^n \| (R \bm{a}_i + \bm{t}) - \bm{b}_i \|^2
\end{align}
$$

これは非線形最小二乗問題であり、ガウス・ニュートン法の適用が考えられる。

まず、元々のガウス・ニュートン法について振り返ると、$n$個のベクトル値関数$\bm{f}_i$の総二乗和の最小化

$$
\argmin_{\bm{x}} \sum_{i=1}^n \| \bm{f}_i(\bm{x}) \|^2
$$

という問題に対して、

$$
\bm{x} \leftarrow \bm{x} - (J_F^T J_F)^{-1} J_F^T \bm{F}(\bm{x})
$$

というようにヤコビ行列$J_F$を用いて更新則を反復することによって、最適解$\bm{x}_{opt}$を求めることができた。ただし、ここで登場するベクトル値関数$F$やヤコビ行列$J_F$とは

$$
\bm{F}(\bm{x}) \triangleq
\begin{bmatrix}
\bm{f}_1(\bm{x}) \\
\bm{f}_2(\bm{x}) \\
\vdots \\
\bm{f}_n(\bm{x}) \\
\end{bmatrix}
\qquad
J_F(\bm{x}) \triangleq \dfrac{\partial F}{\partial \bm{x}}(\bm{x}) =
\begin{bmatrix}
J_{\bm{f}_1}(\bm{x}) \\
J_{\bm{f}_2}(\bm{x}) \\
\vdots      \\
J_{\bm{f}_n}(\bm{x})
\end{bmatrix}
\qquad
J_{\bm{f}_i}(\bm{x}) \triangleq \dfrac{\partial \bm{f}_i}{\partial \bm{x}}(\bm{x})
$$

というように、元のベクトル値関数やそのヤコビ行列を縦にスタックしたものである。

しかしながら、今回は式(1)のように$\bm{f}$がリー群$T$の関数であり、そのままではパラメータ数が過剰であり$J_{f_i}$が適切に計算ができない。従って、この場合は$T$の次元で微分を考えるのではなく、**そのときの$T$の周りのリー代数$\bm{\xi}$のもとで微分を考え、その時の最適な$\bm{\xi}$の更新量に対応した$T$の更新を実施**すればよい。つまり、(1)式の更新則は、

$$
T \leftarrow T \oplus \left( - (J_F^T J_F)^{-1} J_F^T \bm{F}(T) \right ) \qquad \left( J_{\bm{f}_i} \triangleq \left.\dfrac{D \bm{f}_i}{D T}\right|_{\bm{\xi}} \right)
$$

とすればよい。$J_{\bm{f}_i}$の具体的な形状については、まず

$$
\begin{align*}
\dfrac{D \bm{f}_i}{D T}
&= \dfrac{D (T \cdot \bm{a}_i - \bm{b}_i)}{D T}
\\
&= \dfrac{D (T \cdot \bm{a}_i)}{D T}
\\
&= \lim_{\bm{\xi} \to \bm{0}} \dfrac{T \operatorname{Exp}(\bm{\xi}) \cdot \bm{a}_i - T \cdot \bm{a}_i}{\bm{\xi}}
\\
&= \lim_{\bm{\xi} \to \bm{0}} \dfrac{T \operatorname{Exp}(\bm{\xi}) \cdot \bm{a}_i - T \cdot \bm{a}_i}{\bm{\xi}}
\\
&= \lim_{\bm{\xi} \to \bm{0}} \dfrac{T (I + [\bm{\xi}]_\wedge) \cdot \bm{a}_i - T \cdot \bm{a}_i}{\bm{\xi}}
\\
&= \lim_{\bm{\xi} \to \bm{0}} \dfrac{T [\bm{\xi}]_\wedge \cdot \bm{a}_i}{\bm{\xi}}
\end{align*}
$$

と計算できる。ここで、$\lim_{\bm{\xi}\to\bm{0}}\operatorname{Exp}(\bm{\xi}) = I + [\bm{\xi}]_\wedge$を用いた。更に分子について、$[\bm{\xi}]_\wedge \triangleq \begin{bmatrix}\bm{\rho}\\\bm{\theta}\end{bmatrix}_\wedge \triangleq \begin{bmatrix}[\bm{\theta}]_\times&\bm{\rho}\\\bm{0}&0\end{bmatrix} \in \mathfrak{se}(3)$と置くと、

$$
\begin{align*}
&T [\bm{\xi}]_\wedge \cdot \bm{a}_i
\\
&= T \begin{bmatrix}[\bm{\theta}]_\times&\bm{\rho}\\\bm{0}&0\end{bmatrix} \begin{bmatrix} \bm{a}_i \\ 1 \end{bmatrix}
\\
&= T ([\bm{\theta}]_\times \bm{a}_i + \bm{\rho})
\\
&= T (\bm{\rho} - [\bm{a}_i]_\times \bm{\theta})
\\
&= T \begin{bmatrix}I&-[\bm{a}_i]_\times\\\bm{0}&0\end{bmatrix} \begin{bmatrix} \bm{\rho} \\ \bm{\theta} \end{bmatrix}
\\
& = T \begin{bmatrix}I&-[\bm{a}_i]_\times\\\bm{0}&0\end{bmatrix} \bm{\xi}
\\
&= \begin{bmatrix}R & -R[\bm{a}_i]_\times \end{bmatrix} \bm{\xi}
\end{align*}
$$

となるため（ここでクロス積の公式($\bm{a}\times\bm{b} = -\bm{b}\times\bm{a}$)を用いた）、

$$
\dfrac{D \bm{f}_i}{D T} = \begin{bmatrix}R & -R[\bm{a}_i]_\times \end{bmatrix} 
$$

を得る。

#### 実装例

実装例に関しては、scomup氏の実装を参照頂きたい。

https://github.com/scomup/MathematicalRobotics/blob/main/mathR/optimization/demo_3d.py

### SE(2)における誤差状態拡張カルマンフィルタを用いた位置姿勢推定

:::message
Joan Solà氏の『[A micro Lie theory for state estimation in robotics](https://arxiv.org/abs/1812.01537)』の"V. LANDMARK-BASED LOCALIZATION AND MAPPING"の内容をベースに解説する。
:::

二次元上を運動するロボットの各時刻$i$でのポーズ$T_i \in \mathrm{SE}(2)$を求めたい。ロボットは、各時刻で相対的に時刻$\Delta\tau$あたり$\bm{u}\triangleq \Delta \tau(\theta_i, \bm{v}_i)^T$で動いていることを搭載したセンサから検出できるが（$\theta$は角速度、$\bm{v}$は並進速度である）、その計測値には正規分布に従った$\bm{w} \sim \mathcal{N}(0, Q)$のノイズが乗っているものとする。また、ロボットは位置$\bm{b}_k \in \mathbb{R}^2$が既知のビーコンからの相対位置をセンサから$\bm{z}_k \in \mathbb{R}^2$として時々取得できて、その計測値には$\bm{n}\sim\mathcal{N}(0,R)$の誤差が乗るものとする。

典型的なローカライズの問題であり、今回は拡張カルマンフィルタ（EKF: Extended Kalman filter）を用いることとする。

#### リー群の誤差共分散行列

まず、カルマンフィルタを適用するにあたっては、推定すべき状態量$T$の確率分布がガウス分布に近似できると仮定して、その誤差共分散行列を$P$と置く必要がある。しかしながら、$T$はリー群でありこのままではパラメータ数が過剰である。従って、$T$の期待値を$\bar{T}$としたときに$T$と$\bar{T}$の差をリー代数で表現したものに対して誤差共分散行列を定義する。つまり、$P=\mathbb{E}[(T\ominus\bar{T})(T\ominus\bar{T})^T]$と定義する。

#### 時間発展モデル

時間更新モデルは、リー代数がリー群の速度を意味するということを思いだすと、$\bm{u}$そのものがリー代数（のベクトル表示）であることが分かり、時刻$\Delta\tau$あたり$\operatorname{Exp}(\bm{u}_i)$のポーズ変化が起きることになる。従って、時間発展モデルは以下の通りである。

$$
T_{k} = f(T_{k-1}, \bm{u}_k, \bm{w}_k) \triangleq T_{k-1} \oplus (\bm{u}_k + \bm{w}_k) \qquad (1)
$$

#### 観測モデル

観測モデルは簡単であり、ポーズ$T^{-1}$を用いて$\bm{b}_k$をポーズ$T$から見た相対位置に変換可能であることから、以下のようになる。

$$
\bm{z}_k = h(T) + \bm{n} \triangleq T^{-1} \cdot \bm{b}_k + \bm{n} = R^T (\bm{b}_k - \bm{t}) + \bm{n} \qquad (2)
$$

ただし、ここで記載の簡略化のため$(\cdot)$は同時座標表現を経由せずに座標変換をする演算として定義している。

#### 予測ステップ

一旦、普通のカルマンフィルタについて振り返る。まず、線形カルマンフィルタでは

$$
\bm{x}_k = F_k \bm{x}_{k-1} + \bm{u}_k + G_k \bm{w}_k
$$

という線形な時間発展に対して、それぞれの状態の周りの摂動が、

$$
\delta\bm{x}_k = F_k \delta\bm{x}_{k-1} + G_k \delta\bm{w}_k
$$

となるため（制御量$\bm{u}$のノイズは$\bm{w}$に含まれるとして摂動を0としている）、

$$
P \leftarrow F_k P F_k^T + G_k Q G_k^T
$$

という誤差共分散行列$P$の更新則が得られるのであった。

これに対して、拡張カルマンフィルタでは

$$
\bm{x}_k = f(\bm{x}_{k-1}, \bm{u}_k, \bm{w}_k)
$$

という非線形の状態方程式が与えられ、これを右辺の状態の周りで線形化すると、

$$
\delta\bm{x}_k = \left.\dfrac{\partial f}{\partial \bm{x}}\right|_{\bm{x}_{k-1}, \bm{u}_k, \bm{w}_k}  \delta\bm{x}_{k-1} + \left.\dfrac{\partial f}{\partial \bm{w}}\right|_{\bm{x}_{k-1}, \bm{u}_k, \bm{w}_k} \delta\bm{w}_k
$$

という摂動の関係が得られることから、線形カルマンフィルタの場合と比較して

$$
\begin{align*}
F_k &\triangleq \left.\dfrac{\partial f}{\partial \bm{x}}\right|_{\bm{x}_{k-1}, \bm{u}_k, \bm{0}}
\\
G_k &\triangleq \left.\dfrac{\partial f}{\partial \bm{w}}\right|_{\bm{x}_{k-1}, \bm{u}_k, \bm{0}}
\end{align*}
$$

とすることで、線形カルマンフィルタと同じ更新則を用いて$P$を更新することができる。（なお、$\bm{w}$はホワイトノイズであるので、ヤコビ行列の引数の$\bm{w}_k$の実現値としては$\bm{0}$を代入する。）

以上の拡張カルマンフィルタと同じ考え方で、我々は時間発展モデルである(1)式を線形化したい。しかしながら、(1)式の$f$はリー群の要素$T$を引数として取る関数であり、上記と全く同じ手法で$f$を$T$で微分すると前述したような過剰なパラメータが発生することになり都合が悪い。そこで、$T$の摂動をリー代数（のベクトル表示）である$\bm{\xi}$を用いて$T \oplus \delta \bm{\xi}$のようにして捉えて、**$T$の近傍におけるリー代数の世界で表現した摂動**を考える。すると、(1)式の$T$近傍でのリー代数における摂動は、

$$
\delta\bm{\xi}_{k+1} = \left.\dfrac{D f}{D T}\right|_{\bm{\xi}_{k-1}, \bm{u}_k, \bm{w}_k} \delta \bm{\xi}_{k-1} + \left.\dfrac{\partial f}{\partial \bm{w}}\right|_{\bm{\xi}_{k-1}, \bm{u}_k, \bm{w}_k} \delta \bm{w}_k \qquad (\bm{\xi}_{k-1} \triangleq \operatorname{Log}(T_{k-1}))
$$

となる。この関係式を拡張カルマンフィルタに採用しよう。すると、リー代数$\xi$が$T$の$T$の周りで表現した誤差を表す状態量となり、

$$
P \leftarrow FPF^T + GQG^T \quad \left( F \triangleq \left.\dfrac{D f}{D T}\right|_{\bm{\xi}_{k-1}, \bm{u}_k, \bm{0}}, G \triangleq \left.\dfrac{\partial f}{\partial \bm{w}}\right|_{\bm{\xi}_{k-1}, \bm{u}_k, \bm{0}} \right)
$$

という更新式が得られる。ここで注意したいのは$P$はもはや$T$そのものの誤差共分散行列ではなく、そのときの$T$の接空間におけるリー代数で表現された誤差共分散行列となっており、$T$の期待値を$\bar{T}$とすれば$P=\mathbb{E}[(T\ominus\bar{T})(T\ominus\bar{T})^T]$ということになる。普通のカルマンフィルタであれば、$P=\mathbb{E}[(\bm{x}-\bar{\bm{x}})(\bm{x}-\bar{\bm{x}})^T]$であり、$\ominus$を導入したことにより普通の場合とのアナロジーが取りやすくなっている。

なお、このように元の状態量に対して偏微分を計算して線形化を施すのではなく、元の状態量の（ノミナル状態との）誤差を状態量と別に見なしてその周りで線形化を施すことでカルマンフィルタを構成するものを、「誤差状態カルマンフィルタ（ESKF: Error state Kalman filer）」ないしは「誤差状態拡張カルマンフィルタ（ES-EKF：Error state extended Kalman filter）」と呼ぶ。

また、推定値$\hat{T}$そのものの時間更新は、時間発展モデルから

$$
\hat{T} \leftarrow \hat{T} \oplus \bm{u}_i
$$

とシンプルに実行すればよい。

以上をまとめると、予測ステップは以下の通りとなる。

$$
\begin{align*}
\hat{T} &\leftarrow \hat{T} \oplus \bm{u}_i
\\
P &\leftarrow FPF^T + GQG^T \quad \left( F \triangleq \left.\dfrac{D f}{D T}\right|_{\bm{\xi}_{k-1}, \bm{u}_k, \bm{0}}, G \triangleq \left.\dfrac{\partial f}{\partial \bm{w}}\right|_{\bm{\xi}_{k-1}, \bm{u}_k, \bm{0}} \right)
\end{align*}
$$

##### ヤコビ行列FとGの閉形式

実際にヤコビ行列を求める場合は、リー群が関わる基本的な微分に関する知識体系とそれを元にした連鎖律を応用することで求めることができ、詳細については[^lie-cheat-sheet][^micro-lie-theory]を参照してほしい。本稿では結論だけ示す。

[^lie-cheat-sheet]: https://raw.githubusercontent.com/artivis/manif/devel/paper/Lie_theory_cheat_sheet.pdf
[^micro-lie-theory]: https://arxiv.org/abs/1812.01537

$F$の具体的な形は

$$
F = \left.\dfrac{D f}{D T}\right|_{\bm{\xi}, \bm{u}, \bm{0}} = \left.\dfrac{D (T \oplus (\bm{u} + \bm{w}))}{D T}\right|_{\bm{\xi}, \bm{u}, \bm{0}} = \mathrm{Ad}_{\operatorname{Exp}(\bm{u})}^{-1}
$$

であり、随伴行列$\mathrm{Ad}_T$は、$T = \begin{bmatrix}R&\bm{t}\\\bm{0}&1\end{bmatrix}$と置くと、

$$
\mathrm{Ad}_T = \begin{bmatrix} R & -[1]_\wedge \bm{t} \\ \bm{0} & 1 \end{bmatrix} \in \mathbb{R}^{3\times3}
$$

である。（注意：$\mathrm{SE}(3)$の場合は、形が異なり$\mathrm{Ad}_T = \begin{bmatrix} R & [\bm{t}]_\wedge R \\ \bm{0} & R \end{bmatrix} \in \mathbb{R}^{6\times6}$となる）

$G$の具体的な形は

$$
G = \dfrac{\partial f}{\partial \bm{u}} = \dfrac{\partial (T \oplus \bm{u})}{\partial \bm{u}} = J_r(\bm{u})
$$

であり、右ヤコビアン$J_r$は、

$$
\begin{align*}
J_r(\bm{\tau}) &\triangleq \dfrac{\partial \operatorname{Exp}(\bm{\tau})}{\partial \bm{\tau}}
\\
&=
\begin{bmatrix}
\sin\theta/\theta & (1-\cos\theta)/\theta & (\theta\rho_1-\rho_2+\rho_2\cos\theta-\rho_1\sin\theta)/\theta^2 \\
(\cos\theta-1)/\theta & \sin\theta/\theta & (\rho_1+\theta\rho_2-\rho_1\cos\theta-\rho_2\sin\theta)/\theta^2 \\
0 & 0 & 1
\end{bmatrix}
\end{align*}
$$

である。（注意：$\mathrm{SE}(3)$の場合はより複雑であり、詳しくは[^micro-lie-theory]のAPPENDIX Dに当たりたい。）

数値計算において特に$\theta$が微小な場合は、テイラー展開を用いて例えば$\theta$を1次の項で打ち切って

$$
J_r(\bm{\tau}) \approx
\begin{bmatrix}
1 & \theta/2 & \theta\rho_1/6 - \rho_2/2 \\
-\theta/2 & 1 & \rho_1/2 + \theta\rho_1/6 \\
0 & 0 & 1
\end{bmatrix} \quad (\theta \approx 0)
$$

と近似することで0除算を回避できる。

#### 更新ステップ

こちらも、まず普通のカルマンフィルタについて振り返る。線形カルマンフィルタでは、

$$
\bm{z}_k = H_k \bm{x}_k + \bm{v}_k
$$

という線形な観測モデルに対して、

$$
\delta \bm{z}_k = H_k \delta \bm{x}_k + \delta \bm{v}_k
$$

という摂動の関係があり、

$$
\begin{align*}
\bm{e}_k &= \bm{z}_k - H_k \hat{\bm{x}}_{k|k-1}
\\
S_k &= R_k + H_k P_{k|k-1}H_k^T
\\
K_k &= P_{k|k-1} H_k^T S_k^{-1}
\\
\hat{\bm{x}}_{k|k} &= \hat{\bm{x}}_{k|k-1} + K_k \bm{e}_k
\\
P_{k|k} &= (I - K_k H_k) P_{k|k-1}
\end{align*}
$$

という更新則が適用できた。

拡張カルマンフィルタにおいては

$$
\bm{z}_k = h(\bm{x}_k) + \bm{v}_k
$$

という非線形の観測方程式が与えられ、右辺の状態で微分を取ることで、

$$
\delta {z}_k = \left.\dfrac{\partial h}{\partial \bm{x}}\right|_{\bm{x}_k, \bm{v}_k} \delta \bm{x}_{k} + \delta \bm{v}_{k}
$$

という摂動の関係が得られることから、線形カルマンフィルタの場合と比較して

$$
H_k \triangleq \left.\dfrac{\partial h}{\partial \bm{x}}\right|_{\bm{x}_k, \bm{0}}
$$

とすることで、同じ更新則が適用できるのであった。

以上の拡張カルマンフィルタと同じ考え方で、我々は観測モデルである(2)式を線形化したい。しかしながら、(2)式の$h$はリー群の要素$T$を引数として取る関数であり、時間発展モデルの線形化のときと同じ問題に直面する。そこで先ほどと同様にして、(2)式の$T$の近傍でのリー代数における摂動を考えると、

$$
\delta z_k = \left.\dfrac{D h}{D T}\right|_{\bm{\xi}} \delta \xi + \delta \bm{n}
$$

となり、この表現において拡張カルマンフィルタと同様の手続きを踏むことが可能となり、

$$
S = R + HPH^T \quad \left( H \triangleq \left.\dfrac{D h}{D T}\right|_{\bm{\xi}} \right)
$$

のようにして、観測残差の共分散$S$が計算できるようになる。

また、この文脈においては$K\bm{e}$はリー代数の世界における修正量を表現することになる。従って、$\hat{T}$そのものの更新式としては

$$
\hat{T} \leftarrow \hat{T} \oplus K \bm{e}
$$

となる。

以上をまとめると、更新ステップは下記のようになる[^diff-with-sola]。

[^diff-with-sola]: [Joan Sola, et al.](https://arxiv.org/abs/1812.01537)では、$P \leftarrow P - KSK^{-1}$の形の更新則を用いているが、これらは非自明なものの等価である。

$$
\begin{align*}
\bm{e} &= \bm{z}_k - \hat{T}^{-1} \cdot \bm{b}_k
\\
S &= R + HPH^T \quad \left( H \triangleq \dfrac{D h}{D T}\right)
\\
K &= P H^T S^{-1}
\\
\hat{T} &\leftarrow \hat{T} \oplus K \bm{e}
\\
P &\leftarrow (I - K H) P
\end{align*}
$$

##### ヤコビ行列Hの閉形式

こちらも詳細については[^lie-cheat-sheet][^micro-lie-theory]を参照してほしい。本稿では結論だけ示す。

$H$の具体的な形は

$$
H = \dfrac{D h}{D T} = \dfrac{D (T^{-1} \cdot \bm{b}_k)}{D T} =
-
\begin{bmatrix}
I & R^T [1]_\wedge (\bm{b}_k - \bm{t})
\end{bmatrix}
$$

である。（注意：$\mathrm{SE}(3)$の場合は、また別の形となる。）

#### 実装例

以上の検討結果をもとに、状態推定をするPythonプログラムは下記のようになる。`class ESEKF(Estimator)`が誤差状態拡張カルマンフィルタの実装部分である。他の手法と比較できるように、普通の拡張カルマンフィルタ(EKF)やアンセンテッドカルマンフィルタ(UKF)の場合も合わせて実装し、結果をプロットした。

https://github.com/sf624/zenn-docs/blob/main/sample_codes/lie-group-for-robotics/se2_kalman_filter.ipynb

下図に実行初期の状態を表示した。左図が軌跡全体、右図がその時刻の平均位置周りの拡大図である。黒色が真値、オレンジ色が観測更新を行わないただのオドメトリ、緑色が通常のEKF、青色が通常のUKF、そして紫色がリー代数空間によるESEKFの結果を示した。点群は、それぞれの1σ範囲に含まれる状態量が一様分布だと仮定してサンプリングしたもので、確率分布の広がりを可視化するために表示している。

ESEKFについては、実行直後のまだ不確かさが大きなときに最も違いが顕著に出ており、リー代数上で正規分布を仮定しているため、その指数写像の結果はリー代数が微小でない場合はこのようなバナナ状の分布になることがある。この方がより実際の確率分布を適切に表現できると考えられる[^banana]。

[^banana]: https://www.roboticsproceedings.org/rss08/p34.pdf

![](/images/lie-group-for-robotics/esekf_1.png =500x)
*リー代数空間での正規分布の誤差を仮定しているESEKF(紫色)については、xy座標に変換するとバナナ状の形状に確率分布が出てくる*

今回のパラメータ設定においては、このような強い非線形性は薄まっていくため、最終的にはどの手法でも下図のように似たような楕円上の確率分布に収まっていくことが確認できる。

![](/images/lie-group-for-robotics/esekf_2.png =500x)
*収束していくと、各手法のどれでも似たような楕円分布形状に落ち着いていく*

## 参考文献

1. Joan Solà氏らによってロボティクスに適用されるリー群の理論がコンパクトに体系化されており、非常に参考になる。
    - Joan Solà, Jeremie Deray, Dinesh Atchuthan, 2021, "[A micro Lie theory for state estimation in robotics](https://arxiv.org/pdf/1812.01537)"
    - J. Deray and J. Solà, 2024, "[Lie theory cheat sheet](https://github.com/artivis/manif/blob/devel/paper/Lie_theory_cheat_sheet.pdf)"
    - Joan Solà, 2021, "[Lie theory for the roboticist](https://www.youtube.com/watch?v=gy8U7S4LWzs)"
    - Joan Solà, 2020, "[Lie thery for the Roboticist](https://www.youtube.com/watch?v=QR1p0Rabuww)" 
    - Joan Solà, 2017, "[Quaternion kinematics for the error-state Kalman filter](https://arxiv.org/abs/1711.02508)" 
      - 上記は以下のように邦訳が公開されている。
        https://www.flight.t.u-tokyo.ac.jp/?p=800

2. scomup氏の記事も分かりやすく、参考にさせていただいた。
    - scomup (Liu Yang), 2024,『[ロボット技術者向け リー群の速習(2) リー群・リー代数を使った3次元剛体変換](https://qiita.com/scomup/items/d82b35aeb1ecabd75e60)』

3. 鏡慎吾、2023年、『[ロボット工学のためのリー群・リー代数入門](https://www.jstage.jst.go.jp/article/jrsj/41/6/41_41_511/_pdf)』

4. 瀬戸将志、2016年、『[拡張現実感のためのSE3補間を用いたローリングシャッターカメラの位置・姿勢推定](https://naist.repo.nii.ac.jp/record/8177/files/R012616.pdf)』

5. Andrew W. Long, Kevin C. Wolfe, Michael J. Mashner, Gregory S. Chirikjian, 2013, "[The Banana Distribution Is Gaussian: A Localization Study with Exponential Coordinates](https://www.roboticsproceedings.org/rss08/p34.pdf)"