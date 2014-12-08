#include "init.h"
#include "elf.h"
#include "machine.h"
#include "fs.h"
#include "libk.h"
#include "sound.h"

Init::Init() : Process("init",nullptr) {
}

long Init::run() {
	/* initialize sound card */
	SoundCard::init();

    SimpleQueue<const char*> argv;
    argv.addTail(K::strdup("shell"));

    execv("shell",&argv,1);

    Debug::shutdown("What?");
    return 0;
}
