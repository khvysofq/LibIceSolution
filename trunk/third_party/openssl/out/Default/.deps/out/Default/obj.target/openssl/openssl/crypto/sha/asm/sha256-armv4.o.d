cmd_out/Default/obj.target/openssl/openssl/crypto/sha/asm/sha256-armv4.o := arm-arago-linux-gnueabi-gcc '-DNO_WINDOWS_BRAINDEATH' '-DPURIFY' '-DMONOLITH' '-DENGINESDIR="/dev/null"' '-DOPENSSLDIR="/etc/ssl"' '-DOPENSSL_BN_ASM_GF2m' '-DOPENSSL_BN_ASM_MONT' '-DGHASH_ASM' '-DAES_ASM' '-DSHA1_ASM' '-DSHA256_ASM' '-DSHA512_ASM' -I. -Iopenssl -Iopenssl/crypto -Iopenssl/crypto/asn1 -Iopenssl/crypto/evp -Iopenssl/crypto/modes -Iopenssl/include  -Wno-unused-value  -MMD -MF out/Default/.deps/out/Default/obj.target/openssl/openssl/crypto/sha/asm/sha256-armv4.o.d.raw  -c -o out/Default/obj.target/openssl/openssl/crypto/sha/asm/sha256-armv4.o openssl/crypto/sha/asm/sha256-armv4.S
out/Default/obj.target/openssl/openssl/crypto/sha/asm/sha256-armv4.o:  \
 openssl/crypto/sha/asm/sha256-armv4.S openssl/crypto/arm_arch.h
openssl/crypto/sha/asm/sha256-armv4.S:
openssl/crypto/arm_arch.h:
