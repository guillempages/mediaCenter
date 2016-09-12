#include "fileutils.h"

void File::init(const std::string& filename) {
    int posPath, posExtension;
    posPath = filename.rfind('/');
    posExtension = filename.rfind('.');

    if (posPath == std::string::npos) {
        path = "";
        posPath = 0;
    } else {
        ++posPath;
        path = filename.substr(0, posPath);
    }
    if (posExtension == std::string::npos) {
        extension = "";
    } else {
        extension = filename.substr(posExtension);
    }

    baseName = filename.substr(posPath, posExtension - posPath);
}

inline File::File(const char* filename) {
    init((std::string) filename);
}

inline File::File(const std::string& filename) {
    init(filename);
}
/**
 * Get the path where the file is, without the file name.
 */
inline std::string File::getPath() const {
    if (path.length() > 0) {
        return path.substr(0,path.length()-1);
    } else {
        return path;
    }
}
/**
 * Get the extension of the file (or empty if the file has no extension).
 */
inline std::string File::getExtension() const {
    if (extension.length() > 0) {
        return extension.substr(1);
    } else {
        return extension;
    }
}
/**
 * Get the filename without any preceding path.
 */
inline std::string File::getBaseName() const {
    return baseName + extension;
}
/**
 * Get the complete filename including all path parts.
 */
inline std::string File::getFullName() const {
    return path + baseName + extension;
}
/**
 * Get the filename without path or extension.
 */
inline std::string File::getStrippedName() const {
    return baseName;
}

inline File::operator const char*() const {
    return getFullName().c_str();
}

inline bool File::operator ==(const File& rvalue) const {
    return getFullName() == rvalue.getFullName();
}

inline bool File::operator ==(const std::string& rvalue) const {
    return getFullName() == rvalue;
}

inline bool File::operator ==(const char* rvalue) const {
    return getFullName() == rvalue;
}
