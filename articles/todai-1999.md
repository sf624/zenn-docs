---
title: "東大1999年 加法定理の証明"
emoji: "💽"
type: "tech"
topics:
published: false
---

## はじめに

東大の加法定理の証明を求める問題は有名で人気です。幾何的に加法定理を示す解法が一般的ですが、図によるちょっと面倒な議論が必要になってきます。

これに対して、オイラーの定理を用いた方法や回転行列による方法も提案されてはいますが、これらは一見すると(2)での証明は簡潔で美しいものの、その引き換えに(1)の定義のほうに以下のような難点を押し付けた格好となり、高校の指導要領を考慮するとかなり厳しいのでないでしょうか。

### オイラーの定理による方法

$$e^{i\theta} = \cos\theta + i\sin\theta$$

として、三角関数を $e^{i\theta}$ の実部と虚部として定義する際、$e^{i\theta}$ がどのように定義されるかを述べなければいけません。この際、指数関数を冪級数として定義する戦略がありますが、収束性の議論が必要になってくるため、高校の指導要領から外れてきます。

### 回転行列による方法

まず

$$R(\theta) = \begin{pmatrix}\cos\theta&-\sin\theta\\\sin\theta&\cos\theta\end{pmatrix}$$

が、幾何的に二次元平面上での回転を与えることを前提とすることは、循環論法に陥るため用いることができません。

なお別の作戦として、回転行列が直行行列である事実から

$$
R^T R = I, \det R = +1
$$

から出発して、

$$
\frac{dR}{d\theta} = JR, R(0)=I, J=\begin{pmatrix}0&-1\\1&0\end{pmatrix}
$$

の常微分方程式の解として $R$ を定義して、その成分から三角関数を定義する方法が考えられますが、ここで$R$ を構成的に行列指数関数となるという議論をして進める場合、やはり収束性の議論から逃れることができません。

（なお、別のアプローチとして $R$ が上記の常微分方程式の解として一意に存在することだけに言及して、具体的な構成については述べずに議論することができますが、これは後述のアプローチと本質的に同じなので省略します。）


## 常微分方程式による定義からの解法

そこで、以下のように常微分方程式として三角関数を定義し、常微分方程式の解の一意性から(1)と(2)を解いてしまうというアプローチを提案します。

### (1) 定義

以下の初期値問題の解として $\cos$ および $\sin$ を定義する。

$$
\cos\theta : \quad f'' = -f, \quad f(0) = 1, \quad f'(0) = 0
$$

$$
\sin\theta : \quad g'' = -g, \quad g(0) = 0, \quad g'(0) = 1
$$

**【定義の well-definedness：解の一意性】**

$u, v$ が同じ初期条件を満たす2つの解であるとし $$w = u - v$$ とおくと、

$$
w'' + w = 0, \quad w(0) = 0, \quad w'(0) = 0
$$

エネルギー関数 $$E(\theta) = w(\theta)^2 + w'(\theta)^2$$ を考えると、

$$
E'(\theta) = 2w w' + 2w' w'' = 2w'(w + w'') = 0
$$

よって $$E$$ は定数。$$E(0) = 0$$ より $$E \equiv 0$$。$$w^2 \geq 0,\ w'^2 \geq 0$$ より $$w \equiv 0$$、すなわち $$u \equiv v$$。 $$\blacksquare$$

***

### (2) 証明

$$\alpha$$ を固定し、$$\beta$$ の関数

$$
w(\beta) = \cos(\alpha + \beta) - \bigl(\cos\alpha\cos\beta - \sin\alpha\sin\beta\bigr)
$$

を考える。

**【$$w$$ が ODE を満たすこと】**

$$\cos(\alpha+\beta)$$ は定義より $$\dfrac{d^2}{d\beta^2}\cos(\alpha+\beta) = -\cos(\alpha+\beta)$$。

$$\cos\alpha,\ \sin\alpha$$ は $$\beta$$ に関して定数であるから、

$$
\frac{d^2}{d\beta^2}\bigl(\cos\alpha\cos\beta - \sin\alpha\sin\beta\bigr)
= -\cos\alpha\cos\beta + \sin\alpha\sin\beta
= -\bigl(\cos\alpha\cos\beta - \sin\alpha\sin\beta\bigr)
$$

以上より $$w'' + w = 0$$。

**【初期条件の確認】**

定義より $$\cos(0)=1,\ \sin(0)=0$$ であるから、

$$
w(0) = \cos\alpha - (\cos\alpha \cdot 1 - \sin\alpha \cdot 0) = 0
$$

また $$\cos' = -\sin$$ であるから（$$\cos$$ の定義より $$f'(0)=0$$ かつ $$f''=-f$$ を繰り返し微分することで確認できる）、

$$
w'(\beta) = -\sin(\alpha+\beta) + \cos\alpha\sin\beta + \sin\alpha\cos\beta
$$

$$
w'(0) = -\sin\alpha + \cos\alpha \cdot 0 + \sin\alpha \cdot 1 = 0
$$

**【結論】**

$$w'' + w = 0,\ w(0) = 0,\ w'(0) = 0$$ より、エネルギー関数

$$
E(\beta) = w(\beta)^2 + w'(\beta)^2
$$

を考えると $$E'(\beta) = 2w'(w + w'') = 0$$。よって $$E \equiv E(0) = 0$$ であり、

$$
w(\beta) \equiv 0
$$

すなわち、

$$
\boxed{\cos(\alpha+\beta) = \cos\alpha\cos\beta - \sin\alpha\sin\beta} \qquad \blacksquare
$$