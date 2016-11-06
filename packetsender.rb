# Documentation: https://github.com/Homebrew/brew/blob/master/docs/Formula-Cookbook.md
#                http://www.rubydoc.info/github/Homebrew/brew/master/Formula
# PLEASE REMOVE ALL GENERATED COMMENTS BEFORE SUBMITTING YOUR PULL REQUEST!

class Qt5Requirement < Requirement
  fatal true

  satisfy :build_env => false do
    @qmake = which('qmake')
    @qmake
  end

  env do
    ENV.append_path 'PATH', @qmake.parent
  end

  def message
    message = <<-EOS.undent
      Homebrew was unable to find an installation of Qt 5, which is required
      to build Packet Sender. If you have Qt 5 installed, make sure that the qmake executable
      is in your path.

    EOS
  end
end

class Packetsender < Formula
  desc "Packet Sender - Send and Receive TCP / UDP packets"
  homepage "https://packetsender.com/"
  url "https://github.com/dannagle/PacketSender/archive/v5.1-sierra1.tar.gz"
  version "1"
  sha256 "7602c421451cc829d4918a7993e4ec875b5a51ac769e4dae0849ffe1adc0a856"

  # depends_on "cmake" => :build
  depends_on "qt5" # if your formula requires any X11/XQuartz components
  #depends_on Qt5Requirement => :recommended

  def install
    # ENV.deparallelize  # if your formula fails when building in parallel

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
