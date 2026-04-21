package NPC21::Modbus;

# "NPC21 Modbus" protocol support routines.  Basic support for NPC21
# user-defined function has been added to MBclient.pm, but this
# module provides more convenience to users.


use strict; no strict 'subs';
use Carp;
use YAML;
use JSON;
use POSIX;
use Data::Dumper;
use Data::HexDump;
use MBclient;


use constant TAG_START => 0;
use constant TAG_UPDATE => 1;
use constant TAG_GETPLACE => 2;
use constant TAG_PLCPROG => 3;
use constant TAG_CONF => 4;
use constant TAG_CONFSTAT => 5;
use constant TAG_CMDS => 8;
use constant TAG_MORE => 10;
use constant TAG_SLOT => 11;
use constant TAG_OTA_BEGIN => 12;
use constant TAG_OTA_WRITE => 13;
use constant TAG_OTA_END => 14;
use constant TAG_LOGGREP => 15;
use constant TAG_LOGMORE => 17;

use constant MAXLEN => 250;  # data only, after fn+tag+len

my $ipaddr_pat = '^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$';
my $ip6addr_pat = '^([\da-f]{1,4}:{1,2})+[\da-f]{1,4}(%\w+)?$';
my $macaddr_pat = '^([\da-f]{1,2}:){5}([\da-f]{1,2})$';


my @children;  # pids of all subprocesses forked in new()

# When HOST is an IPv6 link-local, we have to try at all interfaces
# available.  We do this by forking a child for every interface that
# tries to connect.  Normally only one child will succeed, others will
# timeout and fail.  The winner child returns to the caller with
# MBclient ready for commands, the parent stays silently waiting and
# exits silently after it sees a child exit code of 0.
sub new {
    my $this = shift;
    my $class = ref($this) || $this;
    my $self = bless {}, $class;

    my %args = @_;
    $self->{SLOT} = $args{SLOT};
    $self->{SLOT2} = $args{SLOT2};
    $self->{VERBOSE} = $args{VERBOSE};

    my $host = $args{HOST} // croak ("missing mandatory HOST arg");
    my $need_suffix;
    if ($host =~ $macaddr_pat) {
        $host = ipv6_link_local_from_mac($host);
        if ($args{IFACE}) {
            $host .= "%$args{IFACE}";
        }
        else {
            $need_suffix = 1;
        }
    }
    elsif ($host =~ /^fe80:/i && $host !~ /%/) {
        # link local w/o %iface
        $need_suffix = 1;
    }

    if (!$need_suffix) {
        $self->{MBclient} = my $m = MBclient->new();
        $m->host($host);
        $m->unit_id(255);
        $m->timeout(1) if $args{ABORT}; # if successful, no reply
        $m->timeout(10) if !$args{ABORT};
        local $SIG{ALRM} = sub {die "connect timeout\n"};
        my $oldalarm = alarm(5);
        if (! $m->open()) {  # try connect(2)
            die "connection error\n";
        }
        alarm ($oldalarm);
    }
    else {  # IPv6 link-local with no %iface.  Try all ifaces in parallel
        for my $iface (list_ifaces()) {
            if (my $pid = fork) {
                push @children, $pid;
            }
            elsif (! defined $pid) {
                warn "fork: $!\n";
                kill 9, @children;
                exit 1;
            }
            else {  # child
                $self->{MBclient} = my $m = MBclient->new();
                $m->host("$host%$iface");
                $m->unit_id(255);
                $m->timeout(1) if $args{ABORT};  # if successful, no reply
                $m->timeout(10) if !$args{ABORT};
                local $SIG{ALRM} = sub {exit(33)};
                my $oldalarm = alarm(5);
                if (! $m->open()) {  # try connect(2)
                    exit(33);
                }
                alarm ($oldalarm);
                return $self;  # winner child, go to work
            }
        }
        while ((my $pid = wait()) != -1) {
            if ($? == 0) {  # winner child finished
                kill 9, @children; # clean up the losers
                exit 0;
            }
            next if $? == (33<<8);  # loser child exited after connect(2) fail
            next if $? == (32<<8);  # a loser terminated by SIGPIPE
            next if $? == 9;        # a loser terminated by SIGKILL
            my ($excode, $signal) = ($? >> 8, $? & 0xff);
            if ($excode != 1 && $excode != 255) {  # exit(1) and die
                warn sprintf("unexpected child exit code %d\n", $excode);
            }
            if ($? & 0xff) {
                warn sprintf("child died by signal %d\n", $? & 0xff);
            }
            kill 9, @children; # clean up everybody
            exit ($excode || 1);
        }
        die "$host: Modbus TCP connection failed\n";
    }

    return $self;
}


# list all interfaces where one can expect LogicBox devices
sub list_ifaces {
    my @res;
    for (`ip link ls`) {
        # 2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP mode DEFAULT group default qlen 1000
        if (/^\d+: (\w+):/ ) {
            my $iface = $1;
            next if /NOARP/;
            next unless /,UP,/;
            next unless /MULTICAST/;
            push @res, $iface;
        }
    }
    return @res;
}


# make IPv6 link-local address from EUI-48
sub ipv6_link_local_from_mac {
    my ($macaddr) = @_;
    my @a = split ':', $macaddr;
    my @b = map {hex} @a;
    $b[0] ^= 0x02; # "Modified"
    return sprintf "fe80::%02x%02x:%02xff:fe%02x:%02x%02x", @b;
}


# Given MBclient reply, return error text or "" if no error
# Call like this:  my $err = NPC21::Modbus::error($reply)
sub error {
    my ($reply) = @_;
    if (! defined $reply) {
        return "PLC not responding";
    }
    if ($reply =~ /^\0/) { # err == 0?
        return "";
    }
    # Got reply with non-zero errno.  Return strerror text
    use vars qw($errtext);
    local ($!, $errtext) = unpack("CA*", $reply);
    return $errtext? $errtext: "$!";
}


# preform a single npc21_modbus protocol operation on the base module,
# a module in a slot or a slave device on a bus behind BCMG module.
#
# $tag e.g. NPC21_MODBUS_TAG_OTA_BEGIN
# $req e.g. pack ("N", $image_length)
sub npc21 {
    my $self = shift;
    my ($tag, $req) = @_;
    my $slot = $self->{SLOT};
    my $slot2 = $self->{SLOT2};
    my $m = $self->{MBclient};
    my $verbose = $self->{VERBOSE};
    my $reply;

    if (defined $slot2) {
        my $subreq2 = pack("Ca*", $tag, $req);
        my $subreq = pack("Cca*", TAG_SLOT, $slot2, $subreq2);
        my $req = pack("ca*", $slot, $subreq);
        warn HexDump($req) if $verbose;
        $reply = $m->npc21(TAG_SLOT, $req);
    }
    elsif (defined $slot) {
        my $subreq = pack("Ca*", $tag, $req);
        my $req = pack("ca*", $slot, $subreq);
        warn HexDump($req) if $verbose;
        $reply = $m->npc21(TAG_SLOT, $req);
    }
    else {
        warn HexDump($req) if $verbose;
        $reply = $m->npc21($tag, $req);
    }

    return $reply;
}


sub npc21_len {
    my $self = shift;
    $self->{MBclient}->npc21_len();
}


sub close {
    my $self = shift;
    $self->{MBclient}->close();
}

1;
