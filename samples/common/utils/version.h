// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#include <array>

#ifndef REALSENSE_SAMPLES_VERSION_H
#define REALSENSE_SAMPLES_VERSION_H

#define RS_SAMPLE_MAJOR_VERSION    0
#define RS_SAMPLE_MINOR_VERSION    6
#define RS_SAMPLE_PATCH_VERSION    5
#define RS_SAMPLE_VERSION  (((RS_SAMPLE_MAJOR_VERSION) * 10000) + ((RS_SAMPLE_MINOR_VERSION) * 100) + (RS_SAMPLE_PATCH_VERSION))
#define RS_SAMPLE_VERSION_STR (VAR_ARG_STRING(RS_SAMPLE_MAJOR_VERSION.RS_SAMPLE_MINOR_VERSION.RS_SAMPLE_PATCH_VERSION))

template<unsigned... Is> struct seq {};
template<unsigned N, unsigned... Is>
struct gen_seq : gen_seq<N-1, N-1, Is...> {};
template<unsigned... Is>
struct gen_seq<0, Is...> : seq<Is...> {};

template<unsigned N1, unsigned... I1, unsigned N2, unsigned... I2>
constexpr std::array<char const, N1+N2-1> concat(char const (&a1)[N1], char const (&a2)[N2], seq<I1...>, seq<I2...>)
{
    return {{ a1[I1]..., a2[I2]... }};
}

template<unsigned N1, unsigned N2>
constexpr std::array<char const, N1+N2-1> concat(char const (&a1)[N1], char const (&a2)[N2])
{
    return concat(a1, a2, gen_seq<N1-1> {}, gen_seq<N2> {});
}

#endif //REALSENSE_SAMPLES_VERSION_H
