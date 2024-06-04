import numpy as np
import matplotlib.pyplot as plt
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

def main():
    #matplotlib.use('svg')
    showplots(np.sin, [sine], -4*np.pi, 4*np.pi,"sine_error.svg")
    showplots(np.cos, [cosine], -4*np.pi, 4*np.pi,"cosine_error.svg")
    print(sine(np.pi/4))
    print(cosine(np.pi/4))
    print(sine(np.pi/4)-cosine(np.pi/4))

if __name__ == '__main__':
    main()