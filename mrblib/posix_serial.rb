class PosixSerial
  # Baud rate
  B4800  = 4800
  B9600  = 9600
  B19200  = 19200
  B38400  = 38400
  B57600  = 57600
  B115200 = 115200
  B230400 = 230400

  # Data bit length
  CS5     = 5
  CS6     = 6
  CS7     = 7
  CS8     = 8

  # Stop bit
  STOP1   = 1
  STOP2   = 2

  # Parity
  NONE    = 0
  EVEN    = 1
  ODD     = 2
  
  def self.open(*args, &block)
#    puts "open"
    serial = self.new(*args)
    return serial unless block
    begin
      yield serial
    ensure
      begin
        serial.close
      rescue StandardError
      end
    end
  end

end
