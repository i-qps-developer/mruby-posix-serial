#
# このサンプル動かすためには、build_config.rbに下記のgemを追加したください。
#
#    conf.gem :github => 'ksss/mruby-signal', :branch => 'master'
#
loop = true
serial = nil
#device = "/dev/ttyACM0"
device = "/dev/tty.usbmodem4322"

begin
  serial = PosixSerial.new(device, PosixSerial::B115200, PosixSerial::CS8, PosixSerial::STOP1, PosixSerial::NONE)
  # タイムアウトを1秒にする
  serial.read_timeout(10)
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
#    cc = serial.getc
#    print cc   
  end
ensure
  serial.close
  puts "serial closed"
end
