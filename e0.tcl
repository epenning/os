#!/usr/bin/expect

set timeout 30

expect_after {
    timeout {
        puts "----> timeout <----\r"
        exit
    }
}

spawn qemu-system-x86_64 -nographic --serial mon:stdio -hdc kernel/kernel.img -hdd fat439/user.img -soundhw sb16

expect "shell> "
send "play m.wav 10\r"

expect "shell> "
send "shutdown\r"

expect "*** System Shutdown ***\r"
send \001
send "x"
