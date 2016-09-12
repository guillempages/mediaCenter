#ifndef FILEUTILS_H_
#define FILEUTILS_H_

#include <string>

class File {
private:
    std::string path;
    std::string extension;
    std::string baseName;

    void init(const std::string& filename);

public:
    File(const char* filename);
    File(const std::string& filename);

    /**
     * Get the path where the file is, without the file name.
     */
    std::string getPath() const;
    /**
     * Get the extension of the file (or empty if the file has no extension).
     */
    std::string getExtension() const;
    /**
     * Get the filename without any preceding path.
     */
    std::string getBaseName() const;
    /**
     * Get the complete filename including all path parts.
     */
    std::string getFullName() const;

    /**
     * Get the filename without path or extension.
     */
    std::string getStrippedName() const;

    operator const char*() const;

    bool operator ==(const File& rvalue) const;
    bool operator ==(const std::string& rvalue) const;
    bool operator ==(const char* rvalue) const;
};

#endif /* FILEUTILS_H_ */
