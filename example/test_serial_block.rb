#
# このサンプル動かすためには、build_config.rbに下記のgemを追加したください。
#
#    conf.gem :github => 'ksss/mruby-signal', :branch => 'master'
#
loop = true

Signal.trap(:INT) { |signo|
  loop = false
}

#device = "/dev/ttyACM0"
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

