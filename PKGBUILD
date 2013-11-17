# Maintainer: Ashley Anderson <amanderson@albinoloverats.net>
# Contributor: Ashley Anderson <amanderson@albinoloverats.net>
pkgname=encrypt
pkgver=2013.11
pkgrel=2
pkgdesc="A simple, cross platform, file encryption application---suitable for any modern desktop or mobile operating system. The GUI, either GTK or native, has been desigen to be common across systems and intuitive to use, whilst still providing command-line capabilities for power-users."
url="https://albinoloverats.net/projects/encrypt"
arch=('i686' 'x86_64' 'arm')
license=('GPL3')
depends=('libgcrypt' 'gtk3' 'curl' 'xz')
makedepends=('pkgconfig')

# you shouldn't need to uncomment this as this PKGBUILD file lives in
# the same Git repoository as the source
# source=(https://albinoloverats.net/downloads/encrypt.tar.xz)

build() {
  cd ${startdir}
  make -f Makefile all
}

package() {
  cd ${startdir}
  mkdir -p ${pkgdir}/usr/{bin,share/{encrypt,applications,man/man1,pixmaps,file/magic,bash-completion/completions,zsh/functions/Completion/Unix}} # ,locale/de/LC_MESSAGES}}
  make -f Makefile install PREFIX=${pkgdir}
}
