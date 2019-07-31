#include "Explorer.hpp"
#include <string.h>
#include <strings.h>
#include "PMApp.hpp"


void Explorer::generateFolderView(fs::path path)
{
    PlaylistMakerApp::path = path;
    if (fs::exists(path) && fs::is_directory(path))
    {
        bool hasDotDot = false;
        for (const auto& entry : fs::directory_iterator(path))
		{
			auto filename = entry.path().filename();
            debug(filename.string().c_str());
			if (fs::is_directory(entry.status()))
			{
				ExplorerItem* dir = new FolderItem(entry.path().filename());
                this->addView(dir);
			}
			else if (fs::is_regular_file(entry.status()))
            {
				ExplorerItem* dir = new FileItem(entry.path().filename());
                this->addView(dir);
            }
			else
			{
				ExplorerItem* dir = new ExplorerItem(entry.path().filename().string(), DirEntTypeEnum::Special);
                this->addView(dir);
            }
            if (filename.string() == "..")
            {
                hasDotDot = true;
            }
            
		}
        if (!hasDotDot)
        {
            ExplorerItem* dir = new ExplorerItem("..", DirEntTypeEnum::Special);
            this->addView(dir);
        }
        
        this->sortChildren();
    }
    else
    {
        error("Failed to open dir.");
    }
    
}

void Explorer::sortChildren()
{
    std::sort(children.begin(), children.end(), 
        [](BoxLayoutChild* v1, BoxLayoutChild* v2)
        {
            return ((ExplorerItem*)v1->view)->type < ((ExplorerItem*)v2->view)->type || 
            (((ExplorerItem*)v1->view)->type == ((ExplorerItem*)v2->view)->type && 
            (strcasecmp(((ListItem*)v1->view)->getLabel().c_str(), ((ListItem*)v2->view)->getLabel().c_str()) < 0));
        }
    );
}
View* Explorer::getAtIndex(unsigned index)
{
    return children[index]->view;
}


Explorer::Explorer(string path) : List(), pathItem(new ExplorerItem(path, DirEntTypeEnum::Path))
{
    this->addView(pathItem);
    this->invalidate();
}

Explorer::~Explorer() {}

ExplorerItem::ExplorerItem(string label, DirEntTypeEnum type) :
    ListItem(label),
    type(type)
{
    this->setClickListener([](View *view)
    {
        debug("clicked %s", ((ListItem*)view)->getLabel().c_str());
        fs::path newPath;
        if(((ExplorerItem*)view)->type == DirEntTypeEnum::Special)
        {
            debug("clicked special");
            newPath = PlaylistMakerApp::path.parent_path();
        }
        else
        {
            debug("clicked folder or file");
            newPath = PlaylistMakerApp::path / ((ListItem*)view)->getLabel();
        }
        if(((ExplorerItem*)view)->type != DirEntTypeEnum::File)
        {
            debug("clicked not file");
            PlaylistMakerApp::explorer->setFocusedIndex(0);
            PlaylistMakerApp::explorer->pathItem->setValue(newPath.string());
            Application::requestFocus(PlaylistMakerApp::explorer->pathItem, FocusDirection::NONE);
            
            while(PlaylistMakerApp::explorer->getViewsCount() > 1)
            {
                debug("removing %s", ((ListItem*)PlaylistMakerApp::explorer->getAtIndex(1))->getLabel().c_str());
                PlaylistMakerApp::explorer->removeView(1, true);
                PlaylistMakerApp::explorer->invalidate();
            }
            debug("generate folder");
            PlaylistMakerApp::explorer->generateFolderView(newPath);

        }
    });
}

ExplorerItem::~ExplorerItem()
{
}

void ExplorerItem::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, Style *style, FrameContext *ctx)
{
    unsigned padding = style->Header.padding;
    ListItem::draw(vg, x+32 + padding, y, width-32,  height, style, ctx);
    // Rectangle
    nvgBeginPath(vg);
    
    nvgFillColor(vg, this->getColor());

    nvgRect(vg, x + padding, y + padding, 32, height - padding * 2);
    nvgFill(vg);
}