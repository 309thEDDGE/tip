import numpy as np
import math

class Bits:

    def __init__(self, val):
        self.val = float(val)

    def downshift(self, shift_count):
        self.val = float(int(self.val) >> shift_count)
        return Bits(self.val)

    def upshift(self, shift_count):
        self.val = float(int(self.val) << shift_count)
        return Bits(self.val)

    def mask(self, bit_count):
        self.val = float(int(self.val) & ((1 << bit_count) - 1))
        return Bits(self.val)

    def mask_val(self, val):
        self.val = float(int(self.val) & int(val))
        return Bits(self.val)

    def add(self, val):
        self.val = self.val + float(val)
        return Bits(self.val)

    def multiply(self, val):
        self.val = float(self.val * float(val))
        return Bits(self.val)

    # self.val interpreted as twos-complement integer.
    # downshift by amount to obtain sign bit.
    def as_twos(self, shift_count):
        if int(self.val) >> shift_count:
            a = np.array([int(self.val)], dtype='uint64')
            b = (int(~a[0]) + 1)
            mask = int((1 << (shift_count + 1)) - 1)
            c = b & mask
            self.val = float(-c)
        return Bits(self.val)

def make_bits(bin_string):
     return Bits(int(bin_string, 2))

class Float32GPS:

    def __init__(self):
        self.denom = float(1 << 24)

    def calculate_lthalf_case(self, word1, word2):
        w1 = Bits(word1).mask(7).upshift(16).multiply(1./self.denom)
        w2 = Bits(word2).multiply(1./self.denom).add(w1.val)
        return w2.val

    def calc(self, word1, word2):
        fsign = math.pow(-1.0, Bits(word1).downshift(15).val)
        expon = math.pow(2.0, Bits(word1).downshift(7).mask_val(Bits(1).upshift(8).val-1).val - 128)
        mantissa = 0.5 + Bits(word1).mask_val(Bits(1).upshift(7).val - 1).upshift(16).multiply(1./self.denom).val + Bits(word2).multiply(1./self.denom).val
        return fsign * expon * mantissa

class Float64GPS:

    def __init__(self):
        self.denom = float(1 << 56)

    def calc(self, word1, word2, word3, word4):
   
        sign = math.pow(-1.0, Bits(word1).downshift(15).val)
        expon = math.pow(2.0, Bits(word1).downshift(7).mask_val(Bits(1.0).upshift(8).val - 1.0).val - 128)
        mantissa = (0.5 + Bits(word1).mask_val(Bits(1.0).upshift(7).val - 1.).upshift(48).val/self.denom + 
                    Bits(word2).upshift(32).val/self.denom + Bits(word3).upshift(16).val/self.denom + 
                    word4/self.denom)
        return sign * expon * mantissa

    def calculate_lthalf_case(self, word1, word2, word3, word4):
    #    output_eu[i] = (uint64_t(input_words[4 * i] & ((wide_one_ << 7) - wide_one_)) << 48) / denom +
				#(uint64_t(input_words[4 * i + 1]) << 32) / denom +
				#(uint64_t(input_words[4 * i + 2]) << 16) / denom +
				#input_words[4 * i + 3] / denom;
        return (Bits(word1).mask_val(Bits(1.0).upshift(7).val - 1.).upshift(48).val / self.denom + 
                Bits(word2).upshift(32).val / self.denom + 
                Bits(word3).upshift(16).val / self.denom +
                word4 / self.denom)

class CAPS:

    def __init__(self):
        self.denom = float(1 << 40)

    def calc(self, word1, word2, word3):
  
        sign = math.pow(-1.0, Bits(word3).downshift(15).val)
        expon = math.pow(2.0, Bits(word1).mask_val(Bits(1.0).upshift(8).add(-1.0).val).val - 128.)
        mantissa = (0.5 + Bits(word3).mask_val(Bits(1.0).upshift(15).add(-1.0).val).upshift(24).val / self.denom +
                    Bits(word2).upshift(8).val / self.denom +
                    Bits(word1).downshift(8).val / self.denom)
        return sign * expon * mantissa

    def calculate_lthalf_case(self, word1, word2, word3):
        return (Bits(word3).mask_val(Bits(1.0).upshift(15).add(-1.0).val).upshift(24).val / self.denom +
                Bits(word2).upshift(8).val / self.denom + 
                Bits(word1).downshift(8).val / self.denom)

class Float321750:

    def __init__(self):
        self.denom = float(1 << 23)

    def calc(self, word1, word2):

        dsign = Bits(word1).downshift(15).val;
        expon = Bits(word2).mask_val(Bits(1.0).upshift(7).add(-1.0).val).val
        exp_sign = Bits(word2).downshift(7).mask_val(Bits(1.0).upshift(1).add(-1.0).val).val
        mantissa = (Bits(word1).mask_val(Bits(1.0).upshift(15).add(-1.0).val).upshift(8).val / self.denom + 
                    Bits(word2).downshift(8).val / self.denom)

        if dsign == 1.0:
            mantissa = mantissa - 1.0;
        if exp_sign == 1.0:
            expon = expon - Bits(1.0).upshift(7).val

        return math.pow(2.0, expon) * mantissa

class Float16:

    def __init__(self, *args, **kwargs):
        pass

    def calc(self, word1):
   #     dsign = pow(neg_one, input_words[i] >> 15);
			#expon = pow(sixteen, (input_words[i] & ((wide_one_ << 4) - wide_one_)));
			#mantissa = (input_words[i] >> 4 & ((wide_one_ << 11) - wide_one_));
			#output_eu[i] = dsign * expon * mantissa;
        dsign = math.pow(-1.0, Bits(word1).downshift(15).val)
        expon = math.pow(16.0, Bits(word1).mask_val(Bits(1.0).upshift(4).add(-1.0).val).val)
        mantissa = Bits(word1).downshift(4).mask_val(Bits(1.0).upshift(11).add(-1.0).val).val
        return dsign * expon * mantissa


if __name__ == '__main__':
    #b = Bits(5)
    #print(b.downshift(2).val)
    #b = Bits(5)
    #print(b.mask(4).val)
    #b = make_bits('101')
    #print(b.downshift(2).val)
    #b = make_bits('101')
    #print(b.val)
    #print(b.as_twos(2).val)

    #f = Float32GPS()
    #res = f.calculate_lthalf_case(32, 553)
    #res = f.calc(1432, 44201)
    #print(res)

    #f = Float64GPS()
    #res = f.calc(44906, 8876, 3567, 4096)
    #res = f.calculate_lthalf_case(83, 22106, 2049, 33311)

    #f = CAPS()
    #res = f.calc(44906, 8876, 3567)
    #res = f.calculate_lthalf_case(3072, 897, 4459)

    #f = Float321750()
    #res = f.calc(4476, 27)

    f = Float16()
    res = f.calc(5384)
    print(res)

