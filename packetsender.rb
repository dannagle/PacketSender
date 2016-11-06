# Packet Sender Homebrew install script
# Packet Sender is copyright Dan Nagle and GPL v2 

class Packetsender < Formula
  desc "Packet Sender - Send and Receive TCP / UDP packets"
  homepage "https://packetsender.com/"
  url "https://github.com/dannagle/PacketSender/archive/v5.1-sierra1.tar.gz"
  version "1"
  sha256 "7602c421451cc829d4918a7993e4ec875b5a51ac769e4dae0849ffe1adc0a856"

  depends_on "qt5" # For qmake, tested with Qt 5.7

  def install
    cd "src"
    system "qmake", "PacketSender.pro"
    system "make"
    system "macdeployqt", "PacketSender.app"
    prefix.install "PacketSender.app"
  end

  test do
      system "#{opt_prefix}/Tarsnap.app/Contents/MacOS/PacketSender", "--version"
  end
end
