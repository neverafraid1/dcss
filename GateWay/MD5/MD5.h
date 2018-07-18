//
// Created by wangzhen on 18-6-11.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_MD5_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_MD5_H

#include <string>
#include <cstdint>

class MD5 {
// Methods
public:

    static std::string PrintMD5(uint8_t md5Digest[8]);
    static std::string MD5String(const char* szString);

    MD5() { Init(); }
    void Init();
    void Update(uint8_t* chInput, uint32_t nInputLen);
    void Finalize();
    uint8_t* Digest() { return m_Digest; }

private:

    void Transform(uint8_t* block);
    void Encode(uint8_t* dest, uint32_t* src, uint32_t nLength);
    void Decode(uint32_t* dest, uint8_t* src, uint32_t nLength);

    inline uint32_t rotate_left(uint32_t x, uint32_t n) { return ((x << n) | (x >> (32-n))); }

    inline uint32_t F(uint32_t x, uint32_t y, uint32_t z) { return ((x & y) | (~x & z)); }

    inline uint32_t G(uint32_t x, uint32_t y, uint32_t z) { return ((x & z) | (y & ~z)); }

    inline uint32_t H(uint32_t x, uint32_t y, uint32_t z) { return (x ^ y ^ z); }

    inline uint32_t I(uint32_t x, uint32_t y, uint32_t z) { return (y ^ (x | ~z)); }

    inline void FF(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
    {
        a += F(b, c, d)+x+ac;
        a = rotate_left(a, s);
        a += b;
    }

    inline void GG(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
    {
        a += G(b, c, d)+x+ac;
        a = rotate_left(a, s);
        a += b;
    }

    inline void HH(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
    {
        a += H(b, c, d)+x+ac;
        a = rotate_left(a, s);
        a += b;
    }

    inline void II(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
    {
        a += I(b, c, d)+x+ac;
        a = rotate_left(a, s);
        a += b;
    }

// Data
private:
    uint32_t m_State[4];
    uint32_t m_Count[2];
    uint8_t m_Buffer[64];
    uint8_t m_Digest[16];
    uint8_t m_Finalized;

};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_MD5_H
