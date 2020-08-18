import numpy as np
import math

def get_exp(val):
    return int(math.floor(math.log2(val)))

def get_float32_precision(val):
    range_ = float(math.pow(2,get_exp(val)))
    prec = range_ / 2**23
    return prec

if __name__ == '__main__':

    test_val = np.zeros(100, dtype='float32')
    test_val[0] = 3.1
    prec = get_float32_precision(test_val[0])
    print('\ntest value 3.1 ({:21.20f}), precision {:21.20f}'.format(test_val[0], prec))
    print('exp is {:d}, 3.1 ~= 2**exp * (1 + mantissa) = 2**exp + 2**exp * mantissa\n==> (2**exp * mantissa) in [0, 2**exp)'.format(get_exp(test_val[0])))
    print('For float32, we have 23 bits of precision,\n2**23 steps in [0, 2**exp) or precision of 2**exp/2**23 = {:21.20f}'.format(prec))

    test_val[1] = 3.1 + prec * 0.2
    prec = get_float32_precision(test_val[1])
    print('\ntest value 3.1 + prec*0.2 ({:21.20f}), precision {:21.20f}'.format(test_val[1], prec))
    print('difference from first val = {:21.20f}'.format(test_val[1] - test_val[0]))

    test_val[2] = 3.1 + prec * 0.8
    prec = get_float32_precision(test_val[2])
    print('\ntest value 3.1 + prec*0.8 ({:21.20f}), precision {:21.20f}'.format(test_val[2], prec))
    print('difference from first val = {:21.20f}'.format(test_val[2] - test_val[0]))

    test_val[3] = 3.1 + prec * 0.999999
    prec = get_float32_precision(test_val[3])
    print('\ntest value 3.1 + prec*0.999999 ({:21.20f}), precision {:21.20f}'.format(test_val[3], prec))
    print('difference from first val = {:21.20f}'.format(test_val[3] - test_val[0]))

    test_val[4] = 3.1 + prec * 1.1
    prec = get_float32_precision(test_val[4])
    print('\ntest value 3.1 + prec*1.1 ({:21.20f}), precision {:21.20f}'.format(test_val[4], prec))
    print('difference from first val = {:21.20f}'.format(test_val[4] - test_val[0]))

    print('\nPrecision at given orders of magnitude:')
    for i in range(11):
        print('10^{:d} = {:d} - exponent = {:d}, precision = {:21.20f}'.format(int(i), int(math.pow(10, i)), get_exp(math.pow(10, i)), get_float32_precision(math.pow(10, i))))