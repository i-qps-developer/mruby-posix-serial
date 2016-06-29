# mruby-posix-serial
mrubyのPosix Serialモジュール

## 概要
　Linux系のPosix規格のシリアル通信をmrubyで使えるようにしたモジュールです。

　現在テスト中です。

## テスト環境
　以下の環境でテスト行いました。

　・Raspberry Pi A+ (Raspbian)  
　・Raspberry Pi 2 B+ (Raspbian)  
　・Linux Mint 17.3  
　・Mac OS El Capitan (version 10.11.5)

## インストール
　mrubyのbuild_config.rbに追加してください。

　conf.gem :github => 'i-qps/mruby-posix-serial', :branch => 'master'

## コマンド

| 　　コマンド名   | 　　　　　　説明　　　　　　　　 |  
|:------------- |:----------------------------|  
| open 　　　　　　|　シリアルポートを開く          |  
| close　　　　　　|　シリアルポートを閉じる        |  
| putc 　　　　　　|　１文字送信する               |  
| puts 　　　　　　|　１行送信する                 |  
| getc 　　　　　　|　１文字入力する               |  
| gets 　　　　　　|　１行入力する                 |  
| read_timeout   |　読み込みタイムアウト時間を     |


## 使い方
　サンプルプログラムは、exampleフォルダにあります。  

　・オープンして使う場合  

    loop = true  
    serial = nil  
    device = "/dev/tty.usbmodem4322"  
    
    begin  
      serial = PosixSerial.new(device, PosixSerial::B115200, PosixSerial::CS8, PosixSerial::STOP1, PosixSerial::NONE)  
    rescue => e  
      puts "#{e.backtrace.map{|vv| " " + vv}.join("\n")}"  
    end
    
    Signal.trap(:INT) { |signo|  
      loop = false  
    }  
    
    begin  
      puts "serial start"  
      while loop
        str = serial.gets  
        puts str  
      end  
    ensure
      serial.close
      puts "serial closed"
    end

　・ブロックとして使う場合

    loop = true
    
    Signal.trap(:INT) { |signo|
      loop = false
    }
    
    device = "/dev/tty.usbmodem4342"
    
    begin
      PosixSerial.open(device, PosixSerial::B115200, PosixSerial::CS8, PosixSerial::STOP1, PosixSerial::NONE) { |serial|
        loop = true
        while loop
          str = serial.gets()
          puts str
        end
      }
    rescue => e
      puts e
    end
    puts "serial closed"
