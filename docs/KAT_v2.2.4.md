Spiral-ATCG v2.2-strength KAT generation
rounds=16 block=40 master=32 nonce=16 session=16 rc=BitRev(~x)

=== Scenario TV1: all-zero master/nonce/sid/plaintext ===

--- TV1 policy=BASE rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 00000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = 31b9a3c27b95212dd2ee68a5ef0326eaf377a2e896ff07e76e73bc6cfbdeb1d47cf13bdbb97b7ef2

--- TV1 policy=SAFE rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 00000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = 34785e1658113c1ecc3be9fdef9fe437110a4b79cc7550faab64321615d0aaa940380f0b922d4630

--- TV1 policy=AGGR rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 00000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = d883c9ba2a2c023d16936380acba71c6e9419e0e20191bf9de66a47a8183f6cf44812bf473936db1

--- TV1 policy=NULL rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 00000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = a24193c8a1b5dc1692770bb1d4e2c9634411464b53f561248656c01db186e1e5ddb39703b06ce148
Policy-axis check: PASS; all 4 policy ciphertexts are distinct for TV1

=== Scenario TV2: sequential bytes 0x00..0x67 across all inputs ===

--- TV2 policy=BASE rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
nonce      = 202122232425262728292a2b2c2d2e2f
sid        = 303132333435363738393a3b3c3d3e3f
plaintext  = 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f6061626364656667
ciphertext = b468fea6cbe75789e9c93a498dbcc27a1e277ae122f29b5e4b88f0a48a48794c6f91ead056a4f86d

--- TV2 policy=SAFE rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
nonce      = 202122232425262728292a2b2c2d2e2f
sid        = 303132333435363738393a3b3c3d3e3f
plaintext  = 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f6061626364656667
ciphertext = 3e10c75eec20cbbfb3fc33833171837de21179c3a59e506e55e9e2843d26929bb0c1d951771205c0

--- TV2 policy=AGGR rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
nonce      = 202122232425262728292a2b2c2d2e2f
sid        = 303132333435363738393a3b3c3d3e3f
plaintext  = 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f6061626364656667
ciphertext = bf047e8343f9b010658f495f407d9405e866e235361e0d307ace597ecb417ad25a51044e310c84bd

--- TV2 policy=NULL rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f
nonce      = 202122232425262728292a2b2c2d2e2f
sid        = 303132333435363738393a3b3c3d3e3f
plaintext  = 404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f6061626364656667
ciphertext = 5b0146a57b46addd7c3930d9e968559d99079557058267d03eb8a28a1d3e6999648cf99bda4b5a39
Policy-axis check: PASS; all 4 policy ciphertexts are distinct for TV2

=== Scenario TV3: all-zero session inputs, single-bit plaintext (bit 0 of byte 0) ===

--- TV3 policy=BASE rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 01000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = 184d039202fce54eb3d78eb4714d589491fd4c30d187cb9c101054f8f7eaf2d222a775ee68dcee14

--- TV3 policy=SAFE rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 01000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = 56c46d36f3add41eb730f73296c7383058cd8fbff4c66cca92ad9b2af2c401d2510eee051505bba8

--- TV3 policy=AGGR rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 01000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = 528402df1d97b76e7227e2c33b0f6cebbcc2e5cac1640bcce1aee3aa9ae272fa169de8a878e0f47e

--- TV3 policy=NULL rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = 0000000000000000000000000000000000000000000000000000000000000000
nonce      = 00000000000000000000000000000000
sid        = 00000000000000000000000000000000
plaintext  = 01000000000000000000000000000000000000000000000000000000000000000000000000000000
ciphertext = c477b4d38c88b39572e148fc3cf0ec152852ef1f24fa4edbd23fa706eaa2a452a76b7618850d52cd
Policy-axis check: PASS; all 4 policy ciphertexts are distinct for TV3

=== Scenario TV4: deterministic SplitMix64-derived (master, nonce, sid, plaintext) ===

--- TV4 policy=BASE rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = e8600ddfb6592bcb8b37c0b0fa3362f366959545ce566b829db17b539ade1f95
nonce      = bcece5aa5225d3395ebcb356bfe6dcd5
sid        = 7b0eaed9d0f88d0d4962831cb63c4b42
plaintext  = d7e55daeda457bdb00a95311bcf47ae9a79e32289ed56de20cae6cc6356dcaaf93fb304251f83764
ciphertext = fcb43f0bdf01783d2b38361857335175c7808d9e590a6aeb7fe112b504f42706d07539b9410579ba

--- TV4 policy=SAFE rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = e8600ddfb6592bcb8b37c0b0fa3362f366959545ce566b829db17b539ade1f95
nonce      = bcece5aa5225d3395ebcb356bfe6dcd5
sid        = 7b0eaed9d0f88d0d4962831cb63c4b42
plaintext  = d7e55daeda457bdb00a95311bcf47ae9a79e32289ed56de20cae6cc6356dcaaf93fb304251f83764
ciphertext = 6a2f38de45cd75028335890e7aa5c469500ca19970349844f8d7842bcd69da0c823dfa89cc48b2b7

--- TV4 policy=AGGR rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = e8600ddfb6592bcb8b37c0b0fa3362f366959545ce566b829db17b539ade1f95
nonce      = bcece5aa5225d3395ebcb356bfe6dcd5
sid        = 7b0eaed9d0f88d0d4962831cb63c4b42
plaintext  = d7e55daeda457bdb00a95311bcf47ae9a79e32289ed56de20cae6cc6356dcaaf93fb304251f83764
ciphertext = 8225e080a8086004ebf0c7ea59c81738912d1eaff499e1bbade30567932ca051c1371dc943859a4a

--- TV4 policy=NULL rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = e8600ddfb6592bcb8b37c0b0fa3362f366959545ce566b829db17b539ade1f95
nonce      = bcece5aa5225d3395ebcb356bfe6dcd5
sid        = 7b0eaed9d0f88d0d4962831cb63c4b42
plaintext  = d7e55daeda457bdb00a95311bcf47ae9a79e32289ed56de20cae6cc6356dcaaf93fb304251f83764
ciphertext = d5c8510ea32aaa7a88b52b5d13206f187eb4d7e368751b912e7edf3de93be9c628d26420b7a89e01
Policy-axis check: PASS; all 4 policy ciphertexts are distinct for TV4

=== Scenario TV5: all-0xff master/nonce/sid/plaintext ===

--- TV5 policy=BASE rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
nonce      = ffffffffffffffffffffffffffffffff
sid        = ffffffffffffffffffffffffffffffff
plaintext  = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
ciphertext = f9458ddf74f9ec8cfa483c15857622015270e8017f97e1425b68d4f5d626cdbda82630cbcf536e22

--- TV5 policy=SAFE rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
nonce      = ffffffffffffffffffffffffffffffff
sid        = ffffffffffffffffffffffffffffffff
plaintext  = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
ciphertext = 14c311ff8592568b6955cd3a16f0e2e8a471efa78e470bd9fd0b7c13f9373bdb9e864f953971f4ad

--- TV5 policy=AGGR rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
nonce      = ffffffffffffffffffffffffffffffff
sid        = ffffffffffffffffffffffffffffffff
plaintext  = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
ciphertext = 1aeaae08d27eb35eb6d6a1fc4bb9a5739ef15fffaeccee80c3a527f105429240d0f9e1ff530c5557

--- TV5 policy=NULL rounds=16 rc=BitRev(~x) roundtrip=PASS ---
master     = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
nonce      = ffffffffffffffffffffffffffffffff
sid        = ffffffffffffffffffffffffffffffff
plaintext  = ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
ciphertext = dfc0bbd394759ff5d9d150f03caf4dd75c16951167475506c47f26b5cab2f7399f96bc6ba0e4b664
Policy-axis check: PASS; all 4 policy ciphertexts are distinct for TV5

========================================
ALL 20 KAT ENTRIES PASSED ROUNDTRIP
ALL 5 SCENARIOS PASSED POLICY-DISTINCTNESS CHECK
