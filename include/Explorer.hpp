#pragma once

#include "PMSettings.hpp"
#include "Logger.hpp"
#include <List.hpp>
#include <filesystem>
namespace fs = std::filesystem;

enum class DirEntTypeEnum
{
    Path = -1,
    Special = 0,
    Folder = 1,
    File = 2
};

class ExplorerItem : public ListItem
{
protected:
public:
    DirEntTypeEnum type;
    ExplorerItem(string label, DirEntTypeEnum type);
    ~ExplorerItem();
    void draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, Style *style, FrameContext *ctx) override;
    virtual NVGcolor getColor(){return RGB(200,200,200);}

};

class FolderItem : public ExplorerItem
{
public:
    FolderItem(fs::path label) : ExplorerItem(label.string(), DirEntTypeEnum::Folder){}
    NVGcolor getColor() override {return RGB(0,0,200);}
};

class FileItem : public ExplorerItem
{
private:
    NVGcolor color;
    bool inPlaylist = false;
    bool knownType = false;
public:
    FileItem(fs::path label) : ExplorerItem(label.string(), DirEntTypeEnum::File)
    {
        string pat = label.extension().string().substr(1).c_str();
        transform(pat.begin(), pat.end(), pat.begin(), ::tolower);
        debug("%s: %s, %d, %d", label.string().c_str(), pat, label.has_extension(), PMSettings::FileTypeLookup.count(pat));
        
        if (label.has_extension() && PMSettings::FileTypeLookup.count(pat) == 1)
        {
            color = RGB(0,200,0);
        }
        else
        {
            color = RGB(200,0,0);
        }
    }
    NVGcolor getColor() override {return color;}
};



class Explorer : public List
{
private:
public:
    ExplorerItem* pathItem;
    Explorer(string path);
    ~Explorer();
    void generateFolderView(fs::path path);
    void sortChildren();
    View* getAtIndex(unsigned index);
};

