class Xzpv < Formula
  desc "Cross-platform XZ archiver with visual progress indicator"
  homepage "https://github.com/Grimmslaw/xzpv.git"
  url "file:///tmp/xzpv-local-clean.tar.gz"
  version "1.0.0.local2"
  sha256 "f7586380da9c60788a742a4a34f3c05f9a50f3834b2e13a999f8c522b6da7759"
  license "MIT"

  head "file:///Users/wesrickey/dev/xzpv"

  # build-time dependencies
  depends_on "cmake" => :build

  # runtime dependencies
  depends_on "pv"
  depends_on "xz"

  def install
    system "cmake", "-S", ".", "-B", "build", *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  def post_install
    system "#{bin}/xzpv", "--install"
  end

  def caveats
    <<~EOS
      The default configuration file was automatically deployed to:
        ~/.config/xzpv/xzpv.conf
    EOS
  end

  test do
    system "#{bin}/xzpv", "--version"
  end
end
