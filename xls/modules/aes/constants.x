// Copyright 2022 The XLS Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Constants used throughout AES implementations.

// The Rijndael S-box.
pub const S_BOX = u8[256]:[
    // 00-0f
    u8:0x63, u8:0x7c, u8:0x77, u8:0x7b, u8:0xf2, u8:0x6b, u8:0x6f, u8:0xc5,
    u8:0x30, u8:0x01, u8:0x67, u8:0x2b, u8:0xfe, u8:0xd7, u8:0xab, u8:0x76,
    // 10-1f
    u8:0xca, u8:0x82, u8:0xc9, u8:0x7d, u8:0xfa, u8:0x59, u8:0x47, u8:0xf0,
    u8:0xad, u8:0xd4, u8:0xa2, u8:0xaf, u8:0x9c, u8:0xa4, u8:0x72, u8:0xc0,
    // 20-2f
    u8:0xb7, u8:0xfd, u8:0x93, u8:0x26, u8:0x36, u8:0x3f, u8:0xf7, u8:0xcc,
    u8:0x34, u8:0xa5, u8:0xe5, u8:0xf1, u8:0x71, u8:0xd8, u8:0x31, u8:0x15,
    // 30-3f
    u8:0x04, u8:0xc7, u8:0x23, u8:0xc3, u8:0x18, u8:0x96, u8:0x05, u8:0x9a,
    u8:0x07, u8:0x12, u8:0x80, u8:0xe2, u8:0xeb, u8:0x27, u8:0xb2, u8:0x75,
    // 40-4f
    u8:0x09, u8:0x83, u8:0x2c, u8:0x1a, u8:0x1b, u8:0x6e, u8:0x5a, u8:0xa0,
    u8:0x52, u8:0x3b, u8:0xd6, u8:0xb3, u8:0x29, u8:0xe3, u8:0x2f, u8:0x84,
    // 50-5f
    u8:0x53, u8:0xd1, u8:0x00, u8:0xed, u8:0x20, u8:0xfc, u8:0xb1, u8:0x5b,
    u8:0x6a, u8:0xcb, u8:0xbe, u8:0x39, u8:0x4a, u8:0x4c, u8:0x58, u8:0xcf,
    // 60-6f
    u8:0xd0, u8:0xef, u8:0xaa, u8:0xfb, u8:0x43, u8:0x4d, u8:0x33, u8:0x85,
    u8:0x45, u8:0xf9, u8:0x02, u8:0x7f, u8:0x50, u8:0x3c, u8:0x9f, u8:0xa8,
    // 70-7f
    u8:0x51, u8:0xa3, u8:0x40, u8:0x8f, u8:0x92, u8:0x9d, u8:0x38, u8:0xf5,
    u8:0xbc, u8:0xb6, u8:0xda, u8:0x21, u8:0x10, u8:0xff, u8:0xf3, u8:0xd2,
    // 80-8f
    u8:0xcd, u8:0x0c, u8:0x13, u8:0xec, u8:0x5f, u8:0x97, u8:0x44, u8:0x17,
    u8:0xc4, u8:0xa7, u8:0x7e, u8:0x3d, u8:0x64, u8:0x5d, u8:0x19, u8:0x73,
    // 90-9f
    u8:0x60, u8:0x81, u8:0x4f, u8:0xdc, u8:0x22, u8:0x2a, u8:0x90, u8:0x88,
    u8:0x46, u8:0xee, u8:0xb8, u8:0x14, u8:0xde, u8:0x5e, u8:0x0b, u8:0xdb,
    // a0-af
    u8:0xe0, u8:0x32, u8:0x3a, u8:0x0a, u8:0x49, u8:0x06, u8:0x24, u8:0x5c,
    u8:0xc2, u8:0xd3, u8:0xac, u8:0x62, u8:0x91, u8:0x95, u8:0xe4, u8:0x79,
    // b0-bf
    u8:0xe7, u8:0xc8, u8:0x37, u8:0x6d, u8:0x8d, u8:0xd5, u8:0x4e, u8:0xa9,
    u8:0x6c, u8:0x56, u8:0xf4, u8:0xea, u8:0x65, u8:0x7a, u8:0xae, u8:0x08,
    // c0-cf
    u8:0xba, u8:0x78, u8:0x25, u8:0x2e, u8:0x1c, u8:0xa6, u8:0xb4, u8:0xc6,
    u8:0xe8, u8:0xdd, u8:0x74, u8:0x1f, u8:0x4b, u8:0xbd, u8:0x8b, u8:0x8a,
    // d0-df
    u8:0x70, u8:0x3e, u8:0xb5, u8:0x66, u8:0x48, u8:0x03, u8:0xf6, u8:0x0e,
    u8:0x61, u8:0x35, u8:0x57, u8:0xb9, u8:0x86, u8:0xc1, u8:0x1d, u8:0x9e,
    // e0-ef
    u8:0xe1, u8:0xf8, u8:0x98, u8:0x11, u8:0x69, u8:0xd9, u8:0x8e, u8:0x94,
    u8:0x9b, u8:0x1e, u8:0x87, u8:0xe9, u8:0xce, u8:0x55, u8:0x28, u8:0xdf,
    // f0-ff
    u8:0x8c, u8:0xa1, u8:0x89, u8:0x0d, u8:0xbf, u8:0xe6, u8:0x42, u8:0x68,
    u8:0x41, u8:0x99, u8:0x2d, u8:0x0f, u8:0xb0, u8:0x54, u8:0xbb, u8:0x16];

// The Rijndael inverse S-box.
pub const INV_S_BOX = u8[256]:[
    // 00-0f
    u8:0x52, u8:0x09, u8:0x6a, u8:0xd5, u8:0x30, u8:0x36, u8:0xa5, u8:0x38,
    u8:0xbf, u8:0x40, u8:0xa3, u8:0x9e, u8:0x81, u8:0xf3, u8:0xd7, u8:0xfb,
    // 10-1f
    u8:0x7c, u8:0xe3, u8:0x39, u8:0x82, u8:0x9b, u8:0x2f, u8:0xff, u8:0x87,
    u8:0x34, u8:0x8e, u8:0x43, u8:0x44, u8:0xc4, u8:0xde, u8:0xe9, u8:0xcb,
    // 20-2f
    u8:0x54, u8:0x7b, u8:0x94, u8:0x32, u8:0xa6, u8:0xc2, u8:0x23, u8:0x3d,
    u8:0xee, u8:0x4c, u8:0x95, u8:0x0b, u8:0x42, u8:0xfa, u8:0xc3, u8:0x4e,
    // 30-3f
    u8:0x08, u8:0x2e, u8:0xa1, u8:0x66, u8:0x28, u8:0xd9, u8:0x24, u8:0xb2,
    u8:0x76, u8:0x5b, u8:0xa2, u8:0x49, u8:0x6d, u8:0x8b, u8:0xd1, u8:0x25,
    // 40-4f
    u8:0x72, u8:0xf8, u8:0xf6, u8:0x64, u8:0x86, u8:0x68, u8:0x98, u8:0x16,
    u8:0xd4, u8:0xa4, u8:0x5c, u8:0xcc, u8:0x5d, u8:0x65, u8:0xb6, u8:0x92,
    // 50-5f
    u8:0x6c, u8:0x70, u8:0x48, u8:0x50, u8:0xfd, u8:0xed, u8:0xb9, u8:0xda,
    u8:0x5e, u8:0x15, u8:0x46, u8:0x57, u8:0xa7, u8:0x8d, u8:0x9d, u8:0x84,
    // 60-6f
    u8:0x90, u8:0xd8, u8:0xab, u8:0x00, u8:0x8c, u8:0xbc, u8:0xd3, u8:0x0a,
    u8:0xf7, u8:0xe4, u8:0x58, u8:0x05, u8:0xb8, u8:0xb3, u8:0x45, u8:0x06,
    // 70-7f
    u8:0xd0, u8:0x2c, u8:0x1e, u8:0x8f, u8:0xca, u8:0x3f, u8:0x0f, u8:0x02,
    u8:0xc1, u8:0xaf, u8:0xbd, u8:0x03, u8:0x01, u8:0x13, u8:0x8a, u8:0x6b,
    // 80-8f
    u8:0x3a, u8:0x91, u8:0x11, u8:0x41, u8:0x4f, u8:0x67, u8:0xdc, u8:0xea,
    u8:0x97, u8:0xf2, u8:0xcf, u8:0xce, u8:0xf0, u8:0xb4, u8:0xe6, u8:0x73,
    // 90-9f
    u8:0x96, u8:0xac, u8:0x74, u8:0x22, u8:0xe7, u8:0xad, u8:0x35, u8:0x85,
    u8:0xe2, u8:0xf9, u8:0x37, u8:0xe8, u8:0x1c, u8:0x75, u8:0xdf, u8:0x6e,
    // a0-af
    u8:0x47, u8:0xf1, u8:0x1a, u8:0x71, u8:0x1d, u8:0x29, u8:0xc5, u8:0x89,
    u8:0x6f, u8:0xb7, u8:0x62, u8:0x0e, u8:0xaa, u8:0x18, u8:0xbe, u8:0x1b,
    // b0-bf
    u8:0xfc, u8:0x56, u8:0x3e, u8:0x4b, u8:0xc6, u8:0xd2, u8:0x79, u8:0x20,
    u8:0x9a, u8:0xdb, u8:0xc0, u8:0xfe, u8:0x78, u8:0xcd, u8:0x5a, u8:0xf4,
    // c0-cf
    u8:0x1f, u8:0xdd, u8:0xa8, u8:0x33, u8:0x88, u8:0x07, u8:0xc7, u8:0x31,
    u8:0xb1, u8:0x12, u8:0x10, u8:0x59, u8:0x27, u8:0x80, u8:0xec, u8:0x5f,
    // d0-df
    u8:0x60, u8:0x51, u8:0x7f, u8:0xa9, u8:0x19, u8:0xb5, u8:0x4a, u8:0x0d,
    u8:0x2d, u8:0xe5, u8:0x7a, u8:0x9f, u8:0x93, u8:0xc9, u8:0x9c, u8:0xef,
    // e0-ef
    u8:0xa0, u8:0xe0, u8:0x3b, u8:0x4d, u8:0xae, u8:0x2a, u8:0xf5, u8:0xb0,
    u8:0xc8, u8:0xeb, u8:0xbb, u8:0x3c, u8:0x83, u8:0x53, u8:0x99, u8:0x61,
    // f0-ff
    u8:0x17, u8:0x2b, u8:0x04, u8:0x7e, u8:0xba, u8:0x77, u8:0xd6, u8:0x26,
    u8:0xe1, u8:0x69, u8:0x14, u8:0x63, u8:0x55, u8:0x21, u8:0x0c, u8:0x7d];

// The AES round constants; hardcoded rather than derived.
pub const R_CON = u32[10]:[0x01000000, 0x02000000, 0x04000000, 0x08000000,
                           0x10000000, 0x20000000, 0x40000000, 0x80000000,
                           0x1b000000, 0x36000000];

// Lookup table for multiplication by 0x2 in GF(2^8).
// Not currently used by gfmul2 (direct computation is used instead), but left
// here in case lookup is found to be faster/otherwise better.
pub const GF_MUL_2_TBL = u8[256]:[
    // 00-0f
    u8:0x00, u8:0x02, u8:0x04, u8:0x06, u8:0x08, u8:0x0a, u8:0x0c, u8:0x0e,
    u8:0x10, u8:0x12, u8:0x14, u8:0x16, u8:0x18, u8:0x1a, u8:0x1c, u8:0x1e,
    // 10-1f
    u8:0x20, u8:0x22, u8:0x24, u8:0x26, u8:0x28, u8:0x2a, u8:0x2c, u8:0x2e,
    u8:0x30, u8:0x32, u8:0x34, u8:0x36, u8:0x38, u8:0x3a, u8:0x3c, u8:0x3e,
    // 20-2f
    u8:0x40, u8:0x42, u8:0x44, u8:0x46, u8:0x48, u8:0x4a, u8:0x4c, u8:0x4e,
    u8:0x50, u8:0x52, u8:0x54, u8:0x56, u8:0x58, u8:0x5a, u8:0x5c, u8:0x5e,
    // 30-3f
    u8:0x60, u8:0x62, u8:0x64, u8:0x66, u8:0x68, u8:0x6a, u8:0x6c, u8:0x6e,
    u8:0x70, u8:0x72, u8:0x74, u8:0x76, u8:0x78, u8:0x7a, u8:0x7c, u8:0x7e,
    // 40-4f
    u8:0x80, u8:0x82, u8:0x84, u8:0x86, u8:0x88, u8:0x8a, u8:0x8c, u8:0x8e,
    u8:0x90, u8:0x92, u8:0x94, u8:0x96, u8:0x98, u8:0x9a, u8:0x9c, u8:0x9e,
    // 50-5f
    u8:0xa0, u8:0xa2, u8:0xa4, u8:0xa6, u8:0xa8, u8:0xaa, u8:0xac, u8:0xae,
    u8:0xb0, u8:0xb2, u8:0xb4, u8:0xb6, u8:0xb8, u8:0xba, u8:0xbc, u8:0xbe,
    // 60-6f
    u8:0xc0, u8:0xc2, u8:0xc4, u8:0xc6, u8:0xc8, u8:0xca, u8:0xcc, u8:0xce,
    u8:0xd0, u8:0xd2, u8:0xd4, u8:0xd6, u8:0xd8, u8:0xda, u8:0xdc, u8:0xde,
    // 70-7f
    u8:0xe0, u8:0xe2, u8:0xe4, u8:0xe6, u8:0xe8, u8:0xea, u8:0xec, u8:0xee,
    u8:0xf0, u8:0xf2, u8:0xf4, u8:0xf6, u8:0xf8, u8:0xfa, u8:0xfc, u8:0xfe,
    // 80-8f
    u8:0x1b, u8:0x19, u8:0x1f, u8:0x1d, u8:0x13, u8:0x11, u8:0x17, u8:0x15,
    u8:0x0b, u8:0x09, u8:0x0f, u8:0x0d, u8:0x03, u8:0x01, u8:0x07, u8:0x05,
    // 90-9f
    u8:0x3b, u8:0x39, u8:0x3f, u8:0x3d, u8:0x33, u8:0x31, u8:0x37, u8:0x35,
    u8:0x2b, u8:0x29, u8:0x2f, u8:0x2d, u8:0x23, u8:0x21, u8:0x27, u8:0x25,
    // a0-af
    u8:0x5b, u8:0x59, u8:0x5f, u8:0x5d, u8:0x53, u8:0x51, u8:0x57, u8:0x55,
    u8:0x4b, u8:0x49, u8:0x4f, u8:0x4d, u8:0x43, u8:0x41, u8:0x47, u8:0x45,
    // b0-bf
    u8:0x7b, u8:0x79, u8:0x7f, u8:0x7d, u8:0x73, u8:0x71, u8:0x77, u8:0x75,
    u8:0x6b, u8:0x69, u8:0x6f, u8:0x6d, u8:0x63, u8:0x61, u8:0x67, u8:0x65,
    // c0-cf
    u8:0x9b, u8:0x99, u8:0x9f, u8:0x9d, u8:0x93, u8:0x91, u8:0x97, u8:0x95,
    u8:0x8b, u8:0x89, u8:0x8f, u8:0x8d, u8:0x83, u8:0x81, u8:0x87, u8:0x85,
    // d0-df
    u8:0xbb, u8:0xb9, u8:0xbf, u8:0xbd, u8:0xb3, u8:0xb1, u8:0xb7, u8:0xb5,
    u8:0xab, u8:0xa9, u8:0xaf, u8:0xad, u8:0xa3, u8:0xa1, u8:0xa7, u8:0xa5,
    // e0-ef
    u8:0xdb, u8:0xd9, u8:0xdf, u8:0xdd, u8:0xd3, u8:0xd1, u8:0xd7, u8:0xd5,
    u8:0xcb, u8:0xc9, u8:0xcf, u8:0xcd, u8:0xc3, u8:0xc1, u8:0xc7, u8:0xc5,
    // f0-ff
    u8:0xfb, u8:0xf9, u8:0xff, u8:0xfd, u8:0xf3, u8:0xf1, u8:0xf7, u8:0xf5,
    u8:0xeb, u8:0xe9, u8:0xef, u8:0xed, u8:0xe3, u8:0xe1, u8:0xe7, u8:0xe5,
];

// Lookup table for multiplication by 0x3 in GF(2^8).
// Not currently used by gfmul3 (direct computation is used instead), but left
// here in case lookup is found to be faster/otherwise better.
pub const GF_MUL_3_TBL = u8[256]:[
    // 00-0f
    u8:0x00, u8:0x03, u8:0x06, u8:0x05, u8:0x0c, u8:0x0f, u8:0x0a, u8:0x09,
    u8:0x18, u8:0x1b, u8:0x1e, u8:0x1d, u8:0x14, u8:0x17, u8:0x12, u8:0x11,
    // 10-1f
    u8:0x30, u8:0x33, u8:0x36, u8:0x35, u8:0x3c, u8:0x3f, u8:0x3a, u8:0x39,
    u8:0x28, u8:0x2b, u8:0x2e, u8:0x2d, u8:0x24, u8:0x27, u8:0x22, u8:0x21,
    // 20-2f
    u8:0x60, u8:0x63, u8:0x66, u8:0x65, u8:0x6c, u8:0x6f, u8:0x6a, u8:0x69,
    u8:0x78, u8:0x7b, u8:0x7e, u8:0x7d, u8:0x74, u8:0x77, u8:0x72, u8:0x71,
    // 30-3f
    u8:0x50, u8:0x53, u8:0x56, u8:0x55, u8:0x5c, u8:0x5f, u8:0x5a, u8:0x59,
    u8:0x48, u8:0x4b, u8:0x4e, u8:0x4d, u8:0x44, u8:0x47, u8:0x42, u8:0x41,
    // 40-4f
    u8:0xc0, u8:0xc3, u8:0xc6, u8:0xc5, u8:0xcc, u8:0xcf, u8:0xca, u8:0xc9,
    u8:0xd8, u8:0xdb, u8:0xde, u8:0xdd, u8:0xd4, u8:0xd7, u8:0xd2, u8:0xd1,
    // 50-5f
    u8:0xf0, u8:0xf3, u8:0xf6, u8:0xf5, u8:0xfc, u8:0xff, u8:0xfa, u8:0xf9,
    u8:0xe8, u8:0xeb, u8:0xee, u8:0xed, u8:0xe4, u8:0xe7, u8:0xe2, u8:0xe1,
    // 60-0f
    u8:0xa0, u8:0xa3, u8:0xa6, u8:0xa5, u8:0xac, u8:0xaf, u8:0xaa, u8:0xa9,
    u8:0xb8, u8:0xbb, u8:0xbe, u8:0xbd, u8:0xb4, u8:0xb7, u8:0xb2, u8:0xb1,
    // 70-7f
    u8:0x90, u8:0x93, u8:0x96, u8:0x95, u8:0x9c, u8:0x9f, u8:0x9a, u8:0x99,
    u8:0x88, u8:0x8b, u8:0x8e, u8:0x8d, u8:0x84, u8:0x87, u8:0x82, u8:0x81,
    // 80-8f
    u8:0x9b, u8:0x98, u8:0x9d, u8:0x9e, u8:0x97, u8:0x94, u8:0x91, u8:0x92,
    u8:0x83, u8:0x80, u8:0x85, u8:0x86, u8:0x8f, u8:0x8c, u8:0x89, u8:0x8a,
    // 90-9f
    u8:0xab, u8:0xa8, u8:0xad, u8:0xae, u8:0xa7, u8:0xa4, u8:0xa1, u8:0xa2,
    u8:0xb3, u8:0xb0, u8:0xb5, u8:0xb6, u8:0xbf, u8:0xbc, u8:0xb9, u8:0xba,
    // a0-af
    u8:0xfb, u8:0xf8, u8:0xfd, u8:0xfe, u8:0xf7, u8:0xf4, u8:0xf1, u8:0xf2,
    u8:0xe3, u8:0xe0, u8:0xe5, u8:0xe6, u8:0xef, u8:0xec, u8:0xe9, u8:0xea,
    // b0-bf
    u8:0xcb, u8:0xc8, u8:0xcd, u8:0xce, u8:0xc7, u8:0xc4, u8:0xc1, u8:0xc2,
    u8:0xd3, u8:0xd0, u8:0xd5, u8:0xd6, u8:0xdf, u8:0xdc, u8:0xd9, u8:0xda,
    // c0-cf
    u8:0x5b, u8:0x58, u8:0x5d, u8:0x5e, u8:0x57, u8:0x54, u8:0x51, u8:0x52,
    u8:0x43, u8:0x40, u8:0x45, u8:0x46, u8:0x4f, u8:0x4c, u8:0x49, u8:0x4a,
    // d0-df
    u8:0x6b, u8:0x68, u8:0x6d, u8:0x6e, u8:0x67, u8:0x64, u8:0x61, u8:0x62,
    u8:0x73, u8:0x70, u8:0x75, u8:0x76, u8:0x7f, u8:0x7c, u8:0x79, u8:0x7a,
    // e0-ef
    u8:0x3b, u8:0x38, u8:0x3d, u8:0x3e, u8:0x37, u8:0x34, u8:0x31, u8:0x32,
    u8:0x23, u8:0x20, u8:0x25, u8:0x26, u8:0x2f, u8:0x2c, u8:0x29, u8:0x2a,
    // f0-ff
    u8:0x0b, u8:0x08, u8:0x0d, u8:0x0e, u8:0x07, u8:0x04, u8:0x01, u8:0x02,
    u8:0x13, u8:0x10, u8:0x15, u8:0x16, u8:0x1f, u8:0x1c, u8:0x19, u8:0x1a,
];

// Lookup table for multiplication by 0x9 in GF(2^8).
pub const GF_MUL_9_TBL = u8[256]:[
    // 00-0f
    u8:0x00, u8:0x09, u8:0x12, u8:0x1b, u8:0x24, u8:0x2d, u8:0x36, u8:0x3f,
    u8:0x48, u8:0x41, u8:0x5a, u8:0x53, u8:0x6c, u8:0x65, u8:0x7e, u8:0x77,
    // 10-1f
    u8:0x90, u8:0x99, u8:0x82, u8:0x8b, u8:0xb4, u8:0xbd, u8:0xa6, u8:0xaf,
    u8:0xd8, u8:0xd1, u8:0xca, u8:0xc3, u8:0xfc, u8:0xf5, u8:0xee, u8:0xe7,
    // 20-2f
    u8:0x3b, u8:0x32, u8:0x29, u8:0x20, u8:0x1f, u8:0x16, u8:0x0d, u8:0x04,
    u8:0x73, u8:0x7a, u8:0x61, u8:0x68, u8:0x57, u8:0x5e, u8:0x45, u8:0x4c,
    // 30-3f
    u8:0xab, u8:0xa2, u8:0xb9, u8:0xb0, u8:0x8f, u8:0x86, u8:0x9d, u8:0x94,
    u8:0xe3, u8:0xea, u8:0xf1, u8:0xf8, u8:0xc7, u8:0xce, u8:0xd5, u8:0xdc,
    // 40-4f
    u8:0x76, u8:0x7f, u8:0x64, u8:0x6d, u8:0x52, u8:0x5b, u8:0x40, u8:0x49,
    u8:0x3e, u8:0x37, u8:0x2c, u8:0x25, u8:0x1a, u8:0x13, u8:0x08, u8:0x01,
    // 50-5f
    u8:0xe6, u8:0xef, u8:0xf4, u8:0xfd, u8:0xc2, u8:0xcb, u8:0xd0, u8:0xd9,
    u8:0xae, u8:0xa7, u8:0xbc, u8:0xb5, u8:0x8a, u8:0x83, u8:0x98, u8:0x91,
    // 60-6f
    u8:0x4d, u8:0x44, u8:0x5f, u8:0x56, u8:0x69, u8:0x60, u8:0x7b, u8:0x72,
    u8:0x05, u8:0x0c, u8:0x17, u8:0x1e, u8:0x21, u8:0x28, u8:0x33, u8:0x3a,
    // 70-7f
    u8:0xdd, u8:0xd4, u8:0xcf, u8:0xc6, u8:0xf9, u8:0xf0, u8:0xeb, u8:0xe2,
    u8:0x95, u8:0x9c, u8:0x87, u8:0x8e, u8:0xb1, u8:0xb8, u8:0xa3, u8:0xaa,
    // 80-8f
    u8:0xec, u8:0xe5, u8:0xfe, u8:0xf7, u8:0xc8, u8:0xc1, u8:0xda, u8:0xd3,
    u8:0xa4, u8:0xad, u8:0xb6, u8:0xbf, u8:0x80, u8:0x89, u8:0x92, u8:0x9b,
    // 90 - 9f
    u8:0x7c, u8:0x75, u8:0x6e, u8:0x67, u8:0x58, u8:0x51, u8:0x4a, u8:0x43,
    u8:0x34, u8:0x3d, u8:0x26, u8:0x2f, u8:0x10, u8:0x19, u8:0x02, u8:0x0b,
    // a0-af
    u8:0xd7, u8:0xde, u8:0xc5, u8:0xcc, u8:0xf3, u8:0xfa, u8:0xe1, u8:0xe8,
    u8:0x9f, u8:0x96, u8:0x8d, u8:0x84, u8:0xbb, u8:0xb2, u8:0xa9, u8:0xa0,
    // b0-bf
    u8:0x47, u8:0x4e, u8:0x55, u8:0x5c, u8:0x63, u8:0x6a, u8:0x71, u8:0x78,
    u8:0x0f, u8:0x06, u8:0x1d, u8:0x14, u8:0x2b, u8:0x22, u8:0x39, u8:0x30,
    // c0-cf
    u8:0x9a, u8:0x93, u8:0x88, u8:0x81, u8:0xbe, u8:0xb7, u8:0xac, u8:0xa5,
    u8:0xd2, u8:0xdb, u8:0xc0, u8:0xc9, u8:0xf6, u8:0xff, u8:0xe4, u8:0xed,
    // d0-df
    u8:0x0a, u8:0x03, u8:0x18, u8:0x11, u8:0x2e, u8:0x27, u8:0x3c, u8:0x35,
    u8:0x42, u8:0x4b, u8:0x50, u8:0x59, u8:0x66, u8:0x6f, u8:0x74, u8:0x7d,
    // e0-ef
    u8:0xa1, u8:0xa8, u8:0xb3, u8:0xba, u8:0x85, u8:0x8c, u8:0x97, u8:0x9e,
    u8:0xe9, u8:0xe0, u8:0xfb, u8:0xf2, u8:0xcd, u8:0xc4, u8:0xdf, u8:0xd6,
    // f0-ff
    u8:0x31, u8:0x38, u8:0x23, u8:0x2a, u8:0x15, u8:0x1c, u8:0x07, u8:0x0e,
    u8:0x79, u8:0x70, u8:0x6b, u8:0x62, u8:0x5d, u8:0x54, u8:0x4f, u8:0x46,
];

// Lookup table for multiplication by 0xb in GF(2^8).
pub const GF_MUL_11_TBL = u8[256]:[
    // 00-0f
    u8:0x00, u8:0x0b, u8:0x16, u8:0x1d, u8:0x2c, u8:0x27, u8:0x3a, u8:0x31,
    u8:0x58, u8:0x53, u8:0x4e, u8:0x45, u8:0x74, u8:0x7f, u8:0x62, u8:0x69,
    // 10-1f
    u8:0xb0, u8:0xbb, u8:0xa6, u8:0xad, u8:0x9c, u8:0x97, u8:0x8a, u8:0x81,
    u8:0xe8, u8:0xe3, u8:0xfe, u8:0xf5, u8:0xc4, u8:0xcf, u8:0xd2, u8:0xd9,
    // 20-2f
    u8:0x7b, u8:0x70, u8:0x6d, u8:0x66, u8:0x57, u8:0x5c, u8:0x41, u8:0x4a,
    u8:0x23, u8:0x28, u8:0x35, u8:0x3e, u8:0x0f, u8:0x04, u8:0x19, u8:0x12,
    // 30-3f
    u8:0xcb, u8:0xc0, u8:0xdd, u8:0xd6, u8:0xe7, u8:0xec, u8:0xf1, u8:0xfa,
    u8:0x93, u8:0x98, u8:0x85, u8:0x8e, u8:0xbf, u8:0xb4, u8:0xa9, u8:0xa2,
    // 40-4f
    u8:0xf6, u8:0xfd, u8:0xe0, u8:0xeb, u8:0xda, u8:0xd1, u8:0xcc, u8:0xc7,
    u8:0xae, u8:0xa5, u8:0xb8, u8:0xb3, u8:0x82, u8:0x89, u8:0x94, u8:0x9f,
    // 50-5f
    u8:0x46, u8:0x4d, u8:0x50, u8:0x5b, u8:0x6a, u8:0x61, u8:0x7c, u8:0x77,
    u8:0x1e, u8:0x15, u8:0x08, u8:0x03, u8:0x32, u8:0x39, u8:0x24, u8:0x2f,
    // 60-6f
    u8:0x8d, u8:0x86, u8:0x9b, u8:0x90, u8:0xa1, u8:0xaa, u8:0xb7, u8:0xbc,
    u8:0xd5, u8:0xde, u8:0xc3, u8:0xc8, u8:0xf9, u8:0xf2, u8:0xef, u8:0xe4,
    // 70-7f
    u8:0x3d, u8:0x36, u8:0x2b, u8:0x20, u8:0x11, u8:0x1a, u8:0x07, u8:0x0c,
    u8:0x65, u8:0x6e, u8:0x73, u8:0x78, u8:0x49, u8:0x42, u8:0x5f, u8:0x54,
    // 80-8f
    u8:0xf7, u8:0xfc, u8:0xe1, u8:0xea, u8:0xdb, u8:0xd0, u8:0xcd, u8:0xc6,
    u8:0xaf, u8:0xa4, u8:0xb9, u8:0xb2, u8:0x83, u8:0x88, u8:0x95, u8:0x9e,
    // 90-9f
    u8:0x47, u8:0x4c, u8:0x51, u8:0x5a, u8:0x6b, u8:0x60, u8:0x7d, u8:0x76,
    u8:0x1f, u8:0x14, u8:0x09, u8:0x02, u8:0x33, u8:0x38, u8:0x25, u8:0x2e,
    // a0-af
    u8:0x8c, u8:0x87, u8:0x9a, u8:0x91, u8:0xa0, u8:0xab, u8:0xb6, u8:0xbd,
    u8:0xd4, u8:0xdf, u8:0xc2, u8:0xc9, u8:0xf8, u8:0xf3, u8:0xee, u8:0xe5,
    // b0-bf
    u8:0x3c, u8:0x37, u8:0x2a, u8:0x21, u8:0x10, u8:0x1b, u8:0x06, u8:0x0d,
    u8:0x64, u8:0x6f, u8:0x72, u8:0x79, u8:0x48, u8:0x43, u8:0x5e, u8:0x55,
    // c0-cf
    u8:0x01, u8:0x0a, u8:0x17, u8:0x1c, u8:0x2d, u8:0x26, u8:0x3b, u8:0x30,
    u8:0x59, u8:0x52, u8:0x4f, u8:0x44, u8:0x75, u8:0x7e, u8:0x63, u8:0x68,
    // d0-df
    u8:0xb1, u8:0xba, u8:0xa7, u8:0xac, u8:0x9d, u8:0x96, u8:0x8b, u8:0x80,
    u8:0xe9, u8:0xe2, u8:0xff, u8:0xf4, u8:0xc5, u8:0xce, u8:0xd3, u8:0xd8,
    // e0-ef
    u8:0x7a, u8:0x71, u8:0x6c, u8:0x67, u8:0x56, u8:0x5d, u8:0x40, u8:0x4b,
    u8:0x22, u8:0x29, u8:0x34, u8:0x3f, u8:0x0e, u8:0x05, u8:0x18, u8:0x13,
    // f0-ff
    u8:0xca, u8:0xc1, u8:0xdc, u8:0xd7, u8:0xe6, u8:0xed, u8:0xf0, u8:0xfb,
    u8:0x92, u8:0x99, u8:0x84, u8:0x8f, u8:0xbe, u8:0xb5, u8:0xa8, u8:0xa3,
];

// Lookup table for multiplication by 0xd in GF(2^8).
pub const GF_MUL_13_TBL = u8[256]:[
    // 00-0f
    u8:0x00, u8:0x0d, u8:0x1a, u8:0x17, u8:0x34, u8:0x39, u8:0x2e, u8:0x23,
    u8:0x68, u8:0x65, u8:0x72, u8:0x7f, u8:0x5c, u8:0x51, u8:0x46, u8:0x4b,
    // 10-1f
    u8:0xd0, u8:0xdd, u8:0xca, u8:0xc7, u8:0xe4, u8:0xe9, u8:0xfe, u8:0xf3,
    u8:0xb8, u8:0xb5, u8:0xa2, u8:0xaf, u8:0x8c, u8:0x81, u8:0x96, u8:0x9b,
    // 20-2f
    u8:0xbb, u8:0xb6, u8:0xa1, u8:0xac, u8:0x8f, u8:0x82, u8:0x95, u8:0x98,
    u8:0xd3, u8:0xde, u8:0xc9, u8:0xc4, u8:0xe7, u8:0xea, u8:0xfd, u8:0xf0,
    // 30-3f
    u8:0x6b, u8:0x66, u8:0x71, u8:0x7c, u8:0x5f, u8:0x52, u8:0x45, u8:0x48,
    u8:0x03, u8:0x0e, u8:0x19, u8:0x14, u8:0x37, u8:0x3a, u8:0x2d, u8:0x20,
    // 40-4f
    u8:0x6d, u8:0x60, u8:0x77, u8:0x7a, u8:0x59, u8:0x54, u8:0x43, u8:0x4e,
    u8:0x05, u8:0x08, u8:0x1f, u8:0x12, u8:0x31, u8:0x3c, u8:0x2b, u8:0x26,
    // 50-5f
    u8:0xbd, u8:0xb0, u8:0xa7, u8:0xaa, u8:0x89, u8:0x84, u8:0x93, u8:0x9e,
    u8:0xd5, u8:0xd8, u8:0xcf, u8:0xc2, u8:0xe1, u8:0xec, u8:0xfb, u8:0xf6,
    // 60-6f
    u8:0xd6, u8:0xdb, u8:0xcc, u8:0xc1, u8:0xe2, u8:0xef, u8:0xf8, u8:0xf5,
    u8:0xbe, u8:0xb3, u8:0xa4, u8:0xa9, u8:0x8a, u8:0x87, u8:0x90, u8:0x9d,
    // 70-7f
    u8:0x06, u8:0x0b, u8:0x1c, u8:0x11, u8:0x32, u8:0x3f, u8:0x28, u8:0x25,
    u8:0x6e, u8:0x63, u8:0x74, u8:0x79, u8:0x5a, u8:0x57, u8:0x40, u8:0x4d,
    // 80-8f
    u8:0xda, u8:0xd7, u8:0xc0, u8:0xcd, u8:0xee, u8:0xe3, u8:0xf4, u8:0xf9,
    u8:0xb2, u8:0xbf, u8:0xa8, u8:0xa5, u8:0x86, u8:0x8b, u8:0x9c, u8:0x91,
    // 90-9f
    u8:0x0a, u8:0x07, u8:0x10, u8:0x1d, u8:0x3e, u8:0x33, u8:0x24, u8:0x29,
    u8:0x62, u8:0x6f, u8:0x78, u8:0x75, u8:0x56, u8:0x5b, u8:0x4c, u8:0x41,
    // a0-af
    u8:0x61, u8:0x6c, u8:0x7b, u8:0x76, u8:0x55, u8:0x58, u8:0x4f, u8:0x42,
    u8:0x09, u8:0x04, u8:0x13, u8:0x1e, u8:0x3d, u8:0x30, u8:0x27, u8:0x2a,
    // b0-bf
    u8:0xb1, u8:0xbc, u8:0xab, u8:0xa6, u8:0x85, u8:0x88, u8:0x9f, u8:0x92,
    u8:0xd9, u8:0xd4, u8:0xc3, u8:0xce, u8:0xed, u8:0xe0, u8:0xf7, u8:0xfa,
    // c0-cf
    u8:0xb7, u8:0xba, u8:0xad, u8:0xa0, u8:0x83, u8:0x8e, u8:0x99, u8:0x94,
    u8:0xdf, u8:0xd2, u8:0xc5, u8:0xc8, u8:0xeb, u8:0xe6, u8:0xf1, u8:0xfc,
    // d0-df
    u8:0x67, u8:0x6a, u8:0x7d, u8:0x70, u8:0x53, u8:0x5e, u8:0x49, u8:0x44,
    u8:0x0f, u8:0x02, u8:0x15, u8:0x18, u8:0x3b, u8:0x36, u8:0x21, u8:0x2c,
    // e0-ef
    u8:0x0c, u8:0x01, u8:0x16, u8:0x1b, u8:0x38, u8:0x35, u8:0x22, u8:0x2f,
    u8:0x64, u8:0x69, u8:0x7e, u8:0x73, u8:0x50, u8:0x5d, u8:0x4a, u8:0x47,
    // f0-ff
    u8:0xdc, u8:0xd1, u8:0xc6, u8:0xcb, u8:0xe8, u8:0xe5, u8:0xf2, u8:0xff,
    u8:0xb4, u8:0xb9, u8:0xae, u8:0xa3, u8:0x80, u8:0x8d, u8:0x9a, u8:0x97,
];

// Lookup table for multiplication by 0xe in GF(2^8).
pub const GF_MUL_14_TBL = u8[256]:[
    // 00-0f
    u8:0x00, u8:0x0e, u8:0x1c, u8:0x12, u8:0x38, u8:0x36, u8:0x24, u8:0x2a,
    u8:0x70, u8:0x7e, u8:0x6c, u8:0x62, u8:0x48, u8:0x46, u8:0x54, u8:0x5a,
    // 10-1f
    u8:0xe0, u8:0xee, u8:0xfc, u8:0xf2, u8:0xd8, u8:0xd6, u8:0xc4, u8:0xca,
    u8:0x90, u8:0x9e, u8:0x8c, u8:0x82, u8:0xa8, u8:0xa6, u8:0xb4, u8:0xba,
    // 20-2f
    u8:0xdb, u8:0xd5, u8:0xc7, u8:0xc9, u8:0xe3, u8:0xed, u8:0xff, u8:0xf1,
    u8:0xab, u8:0xa5, u8:0xb7, u8:0xb9, u8:0x93, u8:0x9d, u8:0x8f, u8:0x81,
    // 30-3f
    u8:0x3b, u8:0x35, u8:0x27, u8:0x29, u8:0x03, u8:0x0d, u8:0x1f, u8:0x11,
    u8:0x4b, u8:0x45, u8:0x57, u8:0x59, u8:0x73, u8:0x7d, u8:0x6f, u8:0x61,
    // 40-4f
    u8:0xad, u8:0xa3, u8:0xb1, u8:0xbf, u8:0x95, u8:0x9b, u8:0x89, u8:0x87,
    u8:0xdd, u8:0xd3, u8:0xc1, u8:0xcf, u8:0xe5, u8:0xeb, u8:0xf9, u8:0xf7,
    // 50-5f
    u8:0x4d, u8:0x43, u8:0x51, u8:0x5f, u8:0x75, u8:0x7b, u8:0x69, u8:0x67,
    u8:0x3d, u8:0x33, u8:0x21, u8:0x2f, u8:0x05, u8:0x0b, u8:0x19, u8:0x17,
    // 60-6f
    u8:0x76, u8:0x78, u8:0x6a, u8:0x64, u8:0x4e, u8:0x40, u8:0x52, u8:0x5c,
    u8:0x06, u8:0x08, u8:0x1a, u8:0x14, u8:0x3e, u8:0x30, u8:0x22, u8:0x2c,
    // 70-7f
    u8:0x96, u8:0x98, u8:0x8a, u8:0x84, u8:0xae, u8:0xa0, u8:0xb2, u8:0xbc,
    u8:0xe6, u8:0xe8, u8:0xfa, u8:0xf4, u8:0xde, u8:0xd0, u8:0xc2, u8:0xcc,
    // 80-8f
    u8:0x41, u8:0x4f, u8:0x5d, u8:0x53, u8:0x79, u8:0x77, u8:0x65, u8:0x6b,
    u8:0x31, u8:0x3f, u8:0x2d, u8:0x23, u8:0x09, u8:0x07, u8:0x15, u8:0x1b,
    // 90-9f
    u8:0xa1, u8:0xaf, u8:0xbd, u8:0xb3, u8:0x99, u8:0x97, u8:0x85, u8:0x8b,
    u8:0xd1, u8:0xdf, u8:0xcd, u8:0xc3, u8:0xe9, u8:0xe7, u8:0xf5, u8:0xfb,
    // a0-af
    u8:0x9a, u8:0x94, u8:0x86, u8:0x88, u8:0xa2, u8:0xac, u8:0xbe, u8:0xb0,
    u8:0xea, u8:0xe4, u8:0xf6, u8:0xf8, u8:0xd2, u8:0xdc, u8:0xce, u8:0xc0,
    // b0-bf
    u8:0x7a, u8:0x74, u8:0x66, u8:0x68, u8:0x42, u8:0x4c, u8:0x5e, u8:0x50,
    u8:0x0a, u8:0x04, u8:0x16, u8:0x18, u8:0x32, u8:0x3c, u8:0x2e, u8:0x20,
    // c0-cf
    u8:0xec, u8:0xe2, u8:0xf0, u8:0xfe, u8:0xd4, u8:0xda, u8:0xc8, u8:0xc6,
    u8:0x9c, u8:0x92, u8:0x80, u8:0x8e, u8:0xa4, u8:0xaa, u8:0xb8, u8:0xb6,
    // d0-df
    u8:0x0c, u8:0x02, u8:0x10, u8:0x1e, u8:0x34, u8:0x3a, u8:0x28, u8:0x26,
    u8:0x7c, u8:0x72, u8:0x60, u8:0x6e, u8:0x44, u8:0x4a, u8:0x58, u8:0x56,
    // e0-ef
    u8:0x37, u8:0x39, u8:0x2b, u8:0x25, u8:0x0f, u8:0x01, u8:0x13, u8:0x1d,
    u8:0x47, u8:0x49, u8:0x5b, u8:0x55, u8:0x7f, u8:0x71, u8:0x63, u8:0x6d,
    // f0-ff
    u8:0xd7, u8:0xd9, u8:0xcb, u8:0xc5, u8:0xef, u8:0xe1, u8:0xf3, u8:0xfd,
    u8:0xa7, u8:0xa9, u8:0xbb, u8:0xb5, u8:0x9f, u8:0x91, u8:0x83, u8:0x8d,
];
