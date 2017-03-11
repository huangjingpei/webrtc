#ifndef CODEC_G726_H
#define CODEC_G726_H

#include "telephony.h"

#include "bitstream.h"
#include "private/bitstream.h"
#include "g726.h"
#include "private/g726.h"

#if 0
class g726codec : public IAudioCodec
{
public:
    g726codec();
    virtual ~g726codec();

    enum {
        rate16 = 2,
        rate24 = 3,
        rate32 = 4,
        rate40 = 5,
    };
    virtual int16_t encode(int16_t *speech,
                           int16_t len, int16_t *encoded);
    virtual int16_t decode(int16_t* encoded,
                           int16_t len, int16_t* decoded, int16_t* type);
    virtual int16_t decodeRCU(int16_t* encoded,
                              int16_t len, int16_t* decoded, int16_t* type) {
        return -1;
    };
    virtual int16_t decodePLC(int16_t* decoded, int16_t frames) {
        return -1;
    };
    virtual int16_t decodeInit();
    virtual int16_t addLatePkt(int16_t* encoded, int16_t len) {
        return -1;
    };
    virtual int16_t getMDinfo() {
        return -1;
    };
    virtual int16_t getPitch(int16_t* encoded, int16_t* length) {
        return -1;
    };
    virtual int16_t updBWEst(const int16_t *encoded,
                             int32_t packet_size, uint16_t rtp_seq_number,
                             uint32_t send_ts, uint32_t arr_ts) {
        return -1;
    };
    virtual int16_t getErrorCode() {
        return -1;
    };
private:
    struct g72x_state enc_state;
    int (*enc_routine)(int sample,
                   int in_coding,
                   struct g72x_state *state_ptr);
    int enc_bits;
    int enc_samples_in;

    struct g72x_state dec_state;
    int (*dec_routine)( int code,
                int out_coding,
                struct g72x_state *state_ptr);
    int dec_bits;
    int dec_samples_in;
};

#endif
#endif

