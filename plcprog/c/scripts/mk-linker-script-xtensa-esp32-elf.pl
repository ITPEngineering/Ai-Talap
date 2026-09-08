#!/usr/bin/perl

sub usage {
    die "Usage: mk-linker-script TEXT_START DATA_START BSS_START\n";
}

@ARGV == 3 or usage();
my ($text_start, $data_start, $bss_start) = @ARGV;

while(<DATA>) {
    s,%TEXT_START%,$text_start,;
    s,%DATA_START%,$data_start,;
    s,%BSS_START%,$bss_start,;
    print;
}

exit 0;

__DATA__
/*
 * https://allthingsembedded.com/post/2020-04-11-mastering-the-gnu-linker-script/
 */
/* selected ROM addresses from ../LB340CPU/build/LB340CPU.map */
__absvdi2 = 0x4006387c;
__absvsi2 = 0x40063868;
__adddf3 = 0x40002590;
__addsf3 = 0x400020e8;
__addvdi3 = 0x40002cbc;
__addvsi3 = 0x40002c98;
__ashldi3 = 0x4000c818;
__ashrdi3 = 0x4000c830;
__bswapdi2 = 0x40064b08;
__bswapsi2 = 0x40064ae0;
__clrsbdi2 = 0x40064b7c;
__clrsbsi2 = 0x40064b64;
__clzdi2 = 0x4000ca50;
__clzsi2 = 0x4000c7e8;
__cmpdi2 = 0x40063820;
__ctzdi2 = 0x4000ca64;
__ctzsi2 = 0x4000c7f0;
__divdc3 = 0x400645a4;
__divdf3 = 0x40002954;
__divdi3 = 0x4000ca84;
__divsi3 = 0x4000c7b8;
__eqdf2 = 0x400636a8;
__eqsf2 = 0x40063374;
__extendsfdf2 = 0x40002c34;
__ffsdi2 = 0x4000ca2c;
__ffssi2 = 0x4000c804;
__fixdfdi = 0x40002ac4;
__fixdfsi = 0x40002a78;
__fixsfdi = 0x4000244c;
__fixsfsi = 0x4000240c;
__fixunsdfsi = 0x40002b30;
__fixunssfdi = 0x40002504;
__fixunssfsi = 0x400024ac;
__floatdidf = 0x4000c988;
__floatdisf = 0x4000c8c0;
__floatsidf = 0x4000c944;
__floatsisf = 0x4000c870;
__floatundidf = 0x4000c978;
__floatundisf = 0x4000c8b0;
__floatunsidf = 0x4000c938;
__floatunsisf = 0x4000c864;
__gcc_bcmp = 0x40064a70;
__gedf2 = 0x40063768;
__gesf2 = 0x4006340c;
__gtdf2 = 0x400636dc;
__gtsf2 = 0x400633a0;
__ledf2 = 0x40063704;
__lesf2 = 0x400633c0;
__lshrdi3 = 0x4000c84c;
__ltdf2 = 0x40063790;
__ltsf2 = 0x4006342c;
__moddi3 = 0x4000cd4c;
__modsi3 = 0x4000c7c0;
__muldc3 = 0x40063c90;
__muldf3 = 0x4006358c;
__muldi3 = 0x4000c9fc;
__mulsf3 = 0x400632c8;
__mulsi3 = 0x4000c7b0;
__mulvdi3 = 0x40002d78;
__mulvsi3 = 0x40002d60;
__nedf2 = 0x400636a8;
__negdf2 = 0x400634a0;
__negdi2 = 0x4000ca14;
__negsf2 = 0x400020c0;
__negvdi2 = 0x40002e98;
__negvsi2 = 0x40002e78;
__nesf2 = 0x40063374;
__paritysi2 = 0x40002f3c;
__popcountdi2 = 0x40002ef8;
__popcountsi2 = 0x40002ed0;
__powidf2 = 0x400638e4;
__subdf3 = 0x400026e4;
__subsf3 = 0x400021d0;
__subvdi3 = 0x40002d20;
__subvsi3 = 0x40002cf8;
__swbuf = 0x40058cb4;
__truncdfsf2 = 0x40002b90;
__ucmpdi2 = 0x40063840;
__udiv_w_sdiv = 0x40064bec;
__udivdi3 = 0x4000cff8;
__udivmoddi4 = 0x40064bf4;
__udivsi3 = 0x4000c7c8;
__umoddi3 = 0x4000d280;
__umodsi3 = 0x4000c7d0;
__unorddf2 = 0x400637f4;
__unordsf2 = 0x40063478;

isalnum = 0x40000f04;
isalpha = 0x40000f18;
isascii = 0x4000c20c;
isblank = 0x40000f2c;
iscntrl = 0x40000f50;
isdigit = 0x40000f64;
isgraph = 0x40000f94;
islower = 0x40000f78;
isprint = 0x40000fa8;
ispunct = 0x40000fc0;
isspace = 0x40000fd4;
isupper = 0x40000fe8;
itoa = 0x400566b4;

labs = 0x40056370;
ldiv = 0x40056378;

memccpy = 0x4000c220;
memchr = 0x4000c244;
memcmp = 0x4000c260;
memcpy = 0x4000c2c8;
memmove = 0x4000c3c0;
memrchr = 0x4000c400;
memset = 0x4000c44c;

qsort = 0x40056424;

rand = 0x40001058;
rand_r = 0x400010d4;

srand = 0x40001004;
strcasecmp = 0x400011cc;
strcasestr = 0x40001210;
strcat = 0x4000c518;
strchr = 0x4000c53c;
strcmp = 0x40001274;
strcoll = 0x40001398;
strcpy = 0x400013ac;
strcspn = 0x4000c558;
strdup = 0x4000143c;
strlcat = 0x40001470;
strlcpy = 0x4000c584;
strlen = 0x400014c0;
strlwr = 0x40001524;
strncasecmp = 0x40001550;
strncat = 0x4000c5c4;
strncmp = 0x4000c5f4;
strncpy = 0x400015d4;
strndup = 0x400016b0;
strnlen = 0x4000c628;
strrchr = 0x40001708;
strsep = 0x40001734;
strspn = 0x4000c648;
strstr = 0x4000c674;
strtok_r = 0x4000c70c;
strtol = 0x4005681c;
strtoul = 0x4005692c;
strupr = 0x4000174c;

toascii = 0x4000c720;
tolower = 0x40001868;
toupper = 0x40001884;

utoa = 0x40056258;

SECTIONS
{
  .text %TEXT_START% : {
        *(.theader)
        *(.literal)
        *(.text)
        . = ALIGN(4);
  }

  .data %DATA_START% : {
       *(.dheader)
       *(.data)
       *(.rodata*)
       . = ALIGN(8);
  }

  .bss %BSS_START% : {
       *(.bss)
       . = ALIGN(8);
  }
}
