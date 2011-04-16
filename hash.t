#!/usr/bin/perl

# (C) Kirill A. Korinskiy

# Tests for is_bot module.

###############################################################################

use warnings;
use strict;

use Test::More;

BEGIN { use FindBin; chdir($FindBin::Bin); }

use lib '../../tests/lib';
use Test::Nginx;

###############################################################################

select STDERR; $| = 1;
select STDOUT; $| = 1;

my $t = Test::Nginx->new()->has(qw/http/)->plan(12)
	->write_file_expand('nginx.conf', <<'EOF');

%%TEST_GLOBALS%%

master_process off;
daemon         off;

events {
}

http {
    %%TEST_GLOBALS_HTTP%%

    server {
        listen       127.0.0.1:8080;
        server_name  localhost;

        md5_hash      $hash_md5        $arg_key;
        crc32_hash    $hash_crc32      $arg_key;
        lookup3_hash  $hash_lookup3    $arg_key;

	location /md5 {
	    return 200 "xxx${hash_md5}yyy";
	}

	location /crc32 {
	    return 200 "xxx${hash_crc32}yyy";
	}

	location /lookup3 {
	    return 200 "xxx${hash_lookup3}yyy";
	}

        # substring
        md5_hash      $hash_md5_0_1      0:1 $arg_key;
        md5_hash      $hash_md5_1_3      1:3 $arg_key;

        crc32_hash    $hash_crc32_0_1    0:1 $arg_key;
        crc32_hash    $hash_crc32_1_3    1:3 $arg_key;

        lookup3_hash  $hash_lookup3_0_1  0:1 $arg_key;
        lookup3_hash  $hash_lookup3_1_3  1:3 $arg_key;

	location /md5_0_1 {
	    return 200 "xxx${hash_md5_0_1}yyy";
	}

	location /md5_1_3 {
	    return 200 "xxx${hash_md5_1_3}yyy";
	}

	location /crc32_0_1 {
	    return 200 "xxx${hash_crc32_0_1}yyy";
	}

	location /crc32_1_3 {
	    return 200 "xxx${hash_crc32_1_3}yyy";
	}

	location /lookup3_0_1 {
	    return 200 "xxx${hash_lookup3_0_1}yyy";
	}

	location /lookup3_1_3 {
	    return 200 "xxx${hash_lookup3_1_3}yyy";
	}

        # broken substring
        md5_hash      $hash_md5_70_80      70:80 $arg_key;
        crc32_hash    $hash_crc32_70_80    70:80 $arg_key;
        lookup3_hash  $hash_lookup3_70_80  70:80 $arg_key;

	location /md5_70_80 {
	    return 200 "xxx${hash_md5_70_80}yyy";
	}

	location /crc32_70_80 {
	    return 200 "xxx${hash_crc32_70_80}yyy";
	}

	location /lookup3_70_80 {
	    return 200 "xxx${hash_lookup3_70_80}yyy";
	}

    }
}

EOF

$t->run();

###############################################################################

like(http_get('/md5?key=test'), qr!xxx098F6BCD4621D373CADE4E832627B4F6yyy!m, 'md5 hash');
like(http_get('/crc32?key=test'), qr!xxxD87F7E0Cyyy!m, 'crc32 hash');
like(http_get('/lookup3?key=test'), qr!xxxBE0C1952yyy!m, 'lookup3 hash');

# substring
like(http_get('/md5_0_1?key=test'), qr!xxx0yyy!m, 'md5 0:1');
like(http_get('/md5_1_3?key=test'), qr!xxx98Fyyy!m, 'md5 1:3');

like(http_get('/crc32_0_1?key=test'), qr!xxxDyyy!m, 'crc32 0:1');
like(http_get('/crc32_1_3?key=test'), qr!xxx87Fyyy!m, 'crc32 0:1');

like(http_get('/lookup3_0_1?key=test'), qr!xxxByyy!m, 'lookup3 0:1');
like(http_get('/lookup3_1_3?key=test'), qr!xxxE0Cyyy!m, 'lookup3 0:1');

# broken substring
like(http_get('/md5_70_80?key=test'), qr!xxxyyy!m, 'broken md5 len');
like(http_get('/crc32_70_80?key=test'), qr!xxxyyy!m, 'broken crc32 len');
like(http_get('/lookup3_70_80?key=test'), qr!xxxyyy!m, 'broken lookup3 len');

###############################################################################
