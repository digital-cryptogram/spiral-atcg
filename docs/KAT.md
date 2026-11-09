Spiral-ATCG v2.2.5-feistel-research KAT generation
rounds=16 block=40 master=32 nonce=16 session=16 substrate=Feistel-v2.2.5

=== Scenario TV1: all-zero master/nonce/sid/plaintext ===

--- TV1 policy=BASE rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 00000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = 0dbcc0e9fe2e1cd0c1dbb484d5e66e7cd9f681c51da0933f7fc19c261ddbb5065b8086a88a8f57d4

--- TV1 policy=SAFE rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 00000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = 2feb778f33d686bd76179457a3ba9d5d3f9ca61949a2c6f3321e45398bb9a4f3b48fe2f346045f2a

--- TV1 policy=AGGR rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 00000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = 012cdaac4b95e7c690d9a19848455bf443def3dd3349c8574d9a7887a5285ba0c674a06272de645c

--- TV1 policy=NULL rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 00000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = 704fe233f60f26b7092472d0564ff4104ccda428230512e682069fb4e4092cdbe3bf88add03ab162
Policy-axis check: PASS; all 4 policy ciphertexts are distinct for TV1

=== Scenario TV2: sequential bytes 0x00..0x67 across all inputs ===

--- TV2 policy=BASE rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
nonce      = 202122232425262728292a2b2c2d2e2f
sid        = 303132333435363738393a3b3c3d3e3f
plaintext  = 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f6061626364656667
ciphertext = 5efc085e3a65fdd59694f643d6b482aba697f122df52e0a35b0925ebfae5db61810e9b1896a15120

--- TV2 policy=SAFE rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
nonce      = 202122232425262728292a2b2c2d2e2f
sid        = 303132333435363738393a3b3c3d3e3f
plaintext  = 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f6061626364656667
ciphertext = 12352a3f9b4b019064980e1779711eb1667eaa8be22eab92af485aac816de9ebb8deb3a285be3835

--- TV2 policy=AGGR rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
nonce      = 202122232425262728292a2b2c2d2e2f
sid        = 303132333435363738393a3b3c3d3e3f
plaintext  = 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f6061626364656667
ciphertext = 61adc07636ed58d3bd8f119d917735a5778efefecebd035efcb0e2fe862d585815d1f2d65860d933

--- TV2 policy=NULL rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
nonce      = 202122232425262728292a2b2c2d2e2f
sid        = 303132333435363738393a3b3c3d3e3f
plaintext  = 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f6061626364656667
ciphertext = e0aaeff5d620c262849f2e6a796bb74e61113fe39860d1074b383eff12397b51d29d70286ed35aa4
Policy-axis check: PASS; all 4 policy ciphertexts are distinct for TV2

=== Scenario TV3: all-zero session inputs, single-bit plaintext (bit 0 of byte 0) ===

--- TV3 policy=BASE rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 01000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = fa91c1c1290855b9d3dd470b3ab23d7b6c4911bda745f12d1ce4bf08a1a2dde72302d566e5a45c4c

--- TV3 policy=SAFE rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 01000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = 37f7bcd2b8fb3e974915edcbf683e12ece6ee4e6a001458bbe239fe6252cf98d56ee0b625b1e33bb

--- TV3 policy=AGGR rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 01000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = 714779e01cb1f51ac63f1e2e9dff1b530503acf74e5e67f990caba14149dab7b2f91214d42934a86

--- TV3 policy=NULL rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 01000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = b16b62898670e7dd3807d263529f27ca8dbc3f066a57ff9cb1a7593e85c3a041c24966802f7f7af1
Policy-axis check: PASS; all 4 policy ciphertexts are distinct for TV3

=== Scenario TV4: deterministic SplitMix64-derived (master, nonce, sid, plaintext) ===

--- TV4 policy=BASE rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = e8600ddfb6592bcb8b37c0b0fa3362f366959545ce566b829db17b539ade1f95
nonce      = bcece5aa5225d3395ebcb356bfe6dcd5
sid        = 7b0eaed9d0f88d0d4962831cb63c4b42
plaintext  = d7e55daeda457bdb00a95311bcf47ae9a79e32289ed56de20cae6cc6356dcaaf93fb304251f83764
ciphertext = 943bc1e83a7f53bc3e0e9978bd65244e837e1f6e500a853f01e275c82d41298a524c35e1bfd144dd

--- TV4 policy=SAFE rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = e8600ddfb6592bcb8b37c0b0fa3362f366959545ce566b829db17b539ade1f95
nonce      = bcece5aa5225d3395ebcb356bfe6dcd5
sid        = 7b0eaed9d0f88d0d4962831cb63c4b42
plaintext  = d7e55daeda457bdb00a95311bcf47ae9a79e32289ed56de20cae6cc6356dcaaf93fb304251f83764
ciphertext = 8d49c7341f697fd970d5a72809b5377123fe516897bd987928d61e97d226b1f93b5cdfa2974fb469

--- TV4 policy=AGGR rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = e8600ddfb6592bcb8b37c0b0fa3362f366959545ce566b829db17b539ade1f95
nonce      = bcece5aa5225d3395ebcb356bfe6dcd5
sid        = 7b0eaed9d0f88d0d4962831cb63c4b42
plaintext  = d7e55daeda457bdb00a95311bcf47ae9a79e32289ed56de20cae6cc6356dcaaf93fb304251f83764
ciphertext = 5894869a46c52c56feca93fce451db56db12fa1b2a9358d06743c6fbd0c92887351b71ee27b3beb6

--- TV4 policy=NULL rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = e8600ddfb6592bcb8b37c0b0fa3362f366959545ce566b829db17b539ade1f95
nonce      = bcece5aa5225d3395ebcb356bfe6dcd5
sid        = 7b0eaed9d0f88d0d4962831cb63c4b42
plaintext  = d7e55daeda457bdb00a95311bcf47ae9a79e32289ed56de20cae6cc6356dcaaf93fb304251f83764
ciphertext = 733ebfc221cb31bffc6f993476250a9f3ad40507021aa92fc6522d4f7e1c02653779b182046e544a
Policy-axis check: PASS; all 4 policy ciphertexts are distinct for TV4

=== Scenario TV5: all-0xff master/nonce/sid/plaintext ===

--- TV5 policy=BASE rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
nonce      = ffffffffffffffffffffffffffffffff
sid        = ffffffffffffffffffffffffffffffff
plaintext  = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
ciphertext = 42630e4ac958cf5c40bbb2ddeb8f468c7e7ffbc0ba24b5d975420f5936d5385559a717bedaec78c7

--- TV5 policy=SAFE rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
nonce      = ffffffffffffffffffffffffffffffff
sid        = ffffffffffffffffffffffffffffffff
plaintext  = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
ciphertext = 8c3565b818c776dd3f829c5ceb9f34b30e883c8ac5a5780403ca5ef33feaaf9e7360d13f0f52aaf4

--- TV5 policy=AGGR rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
nonce      = ffffffffffffffffffffffffffffffff
sid        = ffffffffffffffffffffffffffffffff
plaintext  = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
ciphertext = bf8dd2d4f8cc9914ea6188c2e07a7c31dba479a90c296aee53fbd95247adf7fe1322cadad9c9b04a

--- TV5 policy=NULL rounds=16 substrate=Feistel-v2.2.5 roundtrip=PASS ---
master     = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
nonce      = ffffffffffffffffffffffffffffffff
sid        = ffffffffffffffffffffffffffffffff
plaintext  = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
ciphertext = 7262a07b1c601f95509c3f6356f3c5d8ba374caad6138c501fb4458d57c222d8849db14146caba6e
Policy-axis check: PASS; all 4 policy ciphertexts are distinct for TV5

========================================
ALL 20 FROZEN KAT ENTRIES MATCHED
ALL 20 KAT ENTRIES PASSED ROUNDTRIP
ALL 5 SCENARIOS PASSED POLICY-DISTINCTNESS CHECK
