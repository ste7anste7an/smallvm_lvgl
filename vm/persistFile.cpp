/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Copyright 2018 John Maloney, Bernat Romagosa, and Jens Mönig

// persistFile.cpp - Persistent file/non-volatile storage operations

#include <stdio.h>
#include <string.h>

#include "mem.h"
#include "interp.h"
#include "persist.h"

#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP32) || defined(RP2040_PHILHOWER)

#include "fileSys.h"

#define FILE_NAME "/ublockscode"

int fileSystemInitialized = false;
static File codeFile;

static void closeAndOpenCodeFile() {
	if (codeFile) codeFile.close();
	codeFile = myFS.open(FILE_NAME, "a+");
	codeFile.seek(0, SeekEnd);
}

extern "C" void initFileSystem() {
	// Initialize the file system.

	if (fileSystemInitialized) return;

	#if defined(ARDUINO_ARCH_ESP32)
		myFS.begin(true);
	#else
		myFS.begin();
	#endif
	fileSystemInitialized = true;
}

extern "C" int initCodeFile(uint8 *flash, int flashByteCount) {
	// Called at startup on boards that store code in the file system.
	// Initialize the file system, read the code file, and return

	initFileSystem();
	codeFile = myFS.open(FILE_NAME, "r");
	if (!codeFile) clearCodeFile(0);
	// read code file into simulated Flash:
	int bytesRead = codeFile.readBytes((char*) flash, flashByteCount);
	closeAndOpenCodeFile();
	return bytesRead;
}

extern "C" void writeCodeFile(uint8 *code, int byteCount) {
	if (codeFile) codeFile.write(code, byteCount);
	closeAndOpenCodeFile();
}

extern "C" void writeCodeFileWord(int word) {
	if (codeFile) codeFile.write((uint8 *) &word, 4);
}

extern "C" void clearCodeFile(int cycleCount) {
	if (codeFile) codeFile.close();
	codeFile = myFS.open(FILE_NAME, "w"); // truncate file to zero length
	int headerWord = ('S' << 24) | cycleCount; // Header record, version 1
	writeCodeFileWord(headerWord);
	closeAndOpenCodeFile();
}

// File operations for storing system state

extern "C" void createFile(const char *fileName) {
	File file = myFS.open(fileName, "w");
	file.close();
}

extern "C" void deleteFile(const char *fileName) {
	if (fileExists(fileName)) {
		myFS.remove(fileName);
	}
}

extern "C" int fileExists(const char *fileName) {
	File file = myFS.open(fileName, "r");
	if (!file) return false;
	file.close();
	return true;
}

// Code snapshots

extern "C" int hasStartupSnapshot() {
	initFileSystem();
	return fileExists("/startup.ucode");
}

extern "C" void snapshotCodeToFile(char *fileName, int fileNameBytes) {
	char filePath[64]; // local buffer for full file name
	if (fileNameBytes > 62) return; // file name is too long

	// prefix file name with "/" and add null termionator
	// Note: fileName is not null terminated.
	filePath[0] = '/';
	memcpy(&filePath[1], fileName, fileNameBytes);
	filePath[fileNameBytes + 1] = 0; // null terminator

	int codeStoreByteCount = 0;
	uint8_t *codeStoreData = getCodeStore(&codeStoreByteCount);
	if (codeStoreByteCount < 0) return; // code snapshot not supported

	initFileSystem();
	File file = myFS.open(filePath, "w");
	file.write(codeStoreData, codeStoreByteCount);
	file.close();
}

static int isCodeSnapshot(File *file) {
	// Check the integrity of the given code snapshot file. Return true if okay.

	uint8 header[8];

	int fileSize = file->size();
	if (fileSize >= codeStoreSize()) return false; // file too large
	if ((fileSize % 4) != 0) return false; // file size is not a multiple of four bytes

	file->seek(0, SeekSet); // read from start of file
	while (file->available()) {
		file->read(header, 8);
		int recWords = (header[7] << 24) | (header[6] << 16) | (header[5] << 8) | header[4];

		// check the record tag byte
		if ('R' != header[3]) {
			outputString("Bad code snapshot");
			return false;
		}

		// check the record type byte
		switch(header[2]) {
		case 10:
		case 11:
		case 12:
		case 19:
		case 21:
		case 29:
		case 218:
			break; // okay; these are known record types from persist.h RecordType_t
		default:
			outputString("Bad chunk type in code snapshot");
			return false;
		}

		// check the record length
		int bytesToRead = 4 * recWords;
		if (file->available() < bytesToRead) {
			outputString("Incomplete code snapshot");
			return false;
		}

		// skp to the next record
		file->seek(4 * recWords, SeekCur);
	}
	return true;
}

extern "C" void loadCodeSnapshot(char *fileName) {
	#if !defined(USE_CODE_FILE)

		char fullFileName[64]; // local buffer for full file name
		if (strlen(fileName) > 62) return; // file name is too long

		// prefix fullFileName with "/" and add null termionator
		fullFileName[0] = 0;
		strncat(fullFileName, "/", sizeof(fullFileName) - 1);
		strncat(fullFileName, fileName, sizeof(fullFileName) - 1);

		initFileSystem();
		File file = myFS.open(fullFileName, "r");
		if (!isCodeSnapshot(&file)) {
			outputString("Bad code snapshot");
			outputString(fullFileName);
			return;
		}

		// clear code store
		clearPersistentMemory();
		#if defined(ESP8266)
			clearCodeFile(0);
		#endif

		// copy the file into the code store
		uint8 buffer[1024]; // must be a multiple of four
		file.seek(0, SeekSet); // read from start of file
		while (file.available()) {
			int byteCount = file.read(buffer, sizeof(buffer));
			appendToCodeStore(buffer, byteCount);
		}

		// install scripts and start running
		restoreScripts();
		startAll();
	#endif
}

#else // stubs for boards without file systems

extern "C" int hasStartupSnapshot() { return false; }
extern "C" void loadCodeSnapshot(char *fileName) { } // do nothing

#endif
