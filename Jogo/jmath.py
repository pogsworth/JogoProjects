import numpy as np
import matplotlib.pyplot as plt
import struct

def showplots(f,approxlist,a,b,fname):
    x = np.linspace(a,b,1000)
    plt.figure(1)
    plt.subplot(211)
    vfuncs = [np.vectorize(approx) for approx in approxlist]
    for vf in vfuncs:
        plt.plot(x,vf(x),color="green")
    plt.plot(x,f(x),color="red")
    plt.xlim(a,b)
    plt.ylabel('f(x) and approximations fa(x)')
    plt.subplot(212)
    for vf in vfuncs:
        plt.plot(x,f(x)-vf(x))
    plt.xlim(a,b)
    plt.ylabel('error = f(x)-fa(x)')
    plt.xlabel('x')
    plt.show()
    #plt.savefig(fname)

# return sine of an angle between 0..pi/4
def sinp(x):
    # standard taylor
    #coef1 = 1.0
    #coef3 = -.16666666666
    #coef5 = .00833333333

    # fit from -pi/4 to pi/4 degree 5
    coef1 = 0.999994990
    coef3 = -0.166601570
    coef5 = 0.00812149339
    xx = x * x
    return x * (coef1 + xx * (coef3 + xx * coef5))

# return cosine of an angle between 0..pi/4
def cosp(x):
    # standard taylor coefficients
    #coef2 = -0.5
    #coef4 = 0.04166666666
    #coef6 = -0.00138888888

    coef2 = -.499998566
    coef4 = .0416550209
    coef6 = -.00135858439
    xx = x * x
    return 1 + xx * (coef2 + xx * (coef4 + xx * coef6))

# returns residue and modulus of pi/2
def reduce(x):
    InversePeriod =	2 / np.pi
    n = round(x * InversePeriod)
    a = n & 3
    r = x - n * (np.pi / 2)
    return r,a

def sine(x):
    r,a = reduce(x)
    if a == 0: return sinp(r)
    if a == 1: return cosp(r)
    if a == 2: return -sinp(r)
    if a == 3: return -cosp(r)
    return sinp(r)

def cosine(x):
    r,a = reduce(x)
    if a == 0: return cosp(r)
    if a == 1: return -sinp(r)
    if a == 2: return -cosp(r)
    if a == 3: return sinp(r)
    return cosp(x)

ROOT2 = np.sqrt(2.0)

def arctan2(x):
    coef3 = -0.333358505
    coef5 = 0.19639035
    coef7 = -0.210519684
    coef9 = -0.0198107368
    xx = x*x
    return x + x * xx * (coef3 + xx *(coef5 + xx *(coef7 + xx * coef9)))

def arctan1(x):
    # if x > ROOT2 -1:
    #     return np.pi/4 - arctan2((1-x)/(1+x))
    # else:
        return arctan2(x)
    
def arctan0(x):
    if x > 1:
        return np.pi/2 - arctan1(1/x)
    else:
        return arctan1(x)

def arctan(x):
    if x < 0:
        return -arctan0(-x)
    else:
        return arctan0(x)

def arctan_new(x):
    b = np.pi/6
    k = 0.577350269     # tan pi/6 (root3/3)
    b0 = b/2
    k0 = 0.26794919     # tan pi/12
    A = 0.999999020228907
    B = 0.257977658811405
    C = 0.59120450521312
    neg = False
    if x < 0:
        neg = True
        x = abs(x)
    
    comp = False
    if x > 1:
        comp = True
        x = 1/x
    
    hiseg = False
    if x > k0:
        hiseg = True
        x = (x-k)/(1+k*x)
    
    xx = x*x
    a = x*(A + B*xx) / (1 + C*xx)

    if hiseg:
        a += b
    
    if comp:
        a = np.pi /2 - a

    if neg:
        a = -a
    
    return a

def fastlen(x):
    y = 1
    xx=abs(x)
    yy=abs(y)
    a = max(abs(xx),abs(yy))
    b = min(xx,yy)
    return a + b*b/a

def length(x):
    return np.sqrt(x*x+1)

def power(x, n):
    ret = 1
    factor = x
    while n != 0:
        if n & 1:
            ret *= factor
        factor *= factor
        n >>= 1
    return ret

def exp2_old(x):
    roote = 1.2840254167
    c0 = 1.000000034751
    c1 = 0.49999799
    c2 = 0.166704078
    c3 = 0.04136542
    c4 = 0.009419273

    x *= 4
    n = int(x)
    x -= n
    x /= 4
    e = ((((x*c4 + c3)*x + c2)*x + c1)*x + c0)*x + 1
    p = power(roote, n)
    return e * p

def exp_old(x):
#    y = x * 1.442695040889
    if (x<0):
        return 1/exp2(-x)
    else:
        return exp2(x)

def log2_double(x):
    fb = struct.pack('!d', x)
    ib = struct.unpack('!q', fb)[0]
    y = float(ib) * 2.2204460492503130808472633361816e-16
    ib &= 0xfffffffffffff
    ib |= 0x3ff0000000000000
    fi = struct.pack('!q', ib)
    mx = float(struct.unpack('!d', fi)[0])
#    i = mx * (1<<53)
#    y /= float(1<<52)
    #mx *= .5
    n = 1.498030302 * mx + 1.72587999 / (0.3520887068 + mx)
    return y - 1000.651196 - n # * 0.69314718

def log2(x):
    fb = struct.pack('!f', x)
    ib = struct.unpack('!i', fb)[0]
    y = ib * 1.0/(1<<23)
    ib &= 0x7fffff
    ib |= 0x3f000000
    fi = struct.pack('!i', ib)
    mx = struct.unpack('!f', fi)[0]
#    i = mx * (1<<53)
#    y /= float(1<<52)
    #mx *= .5
    n = 1.498030302 * mx + 1.72587999 / (0.3520887068 + mx)
    return y - 124.22551499 - n # * 0.69314718

def log2_004(x):
    mx, ex = np.frexp(x)
    mx *= 2
    ex -= 2
    ex += (-0.34484843*mx + 2.02466578) * mx - 0.67487759
    # ex *= 0.69314718
    return ex

def exp2(x):
    offset = 1.0 if x < 0 else 0.0
    clipp = -126.0 if x < -126 else x
    w = int(clipp)
    z = clipp - w + offset
    v = int((1 << 23) * (clipp + 121.2740575 + 27.7280233 / (4.84252568 - z) - 1.49012907 * z))
    vi = struct.pack('i',v)
    vf = struct.unpack('f',vi)[0]
    return vf

def fastcos(x):
    tp = 1./(2.*3.14159265358)
    x *= tp
    x -= .25 + np.floor(x + .25)
    x *= 16.0 * (abs(x) - .5)
    x += .225 * x * (abs(x) - 1.0)
    return x

def main():
    #matplotlib.use('svg')
#    showplots(np.sin, [sine], -4*np.pi, 4*np.pi,"sine_error.svg")
#    showplots(np.cos, [cosine], -4*np.pi, 4*np.pi,"cosine_error.svg")
    # print(sine(np.pi/4))
    # print(cosine(np.pi/4))
    # print(sine(np.pi/4)-cosine(np.pi/4))
    showplots(np.cos, [fastcos], 1000000, 1000006, "arctan_error.svg")
print(log2(64))

if __name__ == '__main__':
    main()