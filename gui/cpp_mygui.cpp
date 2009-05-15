/*
  OpenMW - The completely unofficial reimplementation of Morrowind
  Copyright (C) 2008  Nicolay Korslund
  Email: < korslund@gmail.com >
  WWW: http://openmw.snaptoad.com/

  This file (cpp_mygui.cpp) is part of the OpenMW package.

  OpenMW is distributed as free software: you can redistribute it
  and/or modify it under the terms of the GNU General Public License
  version 3, as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  version 3 along with this program. If not, see
  http://www.gnu.org/licenses/ .

 */

// TODO: KILLME
std::string cellName;

extern "C" void gui_setCellName(char *str)
{
  cellName = str;
}

// Get the widget type, as a string
extern "C" const char *gui_widgetType(MyGUI::WidgetPtr p)
{ return p->getTypeName().c_str(); }

extern "C" int32_t gui_getHeight(MyGUI::WidgetPtr p)
{
  if(p == NULL) mWindow->getHeight();
  return p->getHeight();
}

extern "C" int32_t gui_getWidth(MyGUI::WidgetPtr p)
{
  if(p == NULL) return mWindow->getWidth();
  return p->getWidth();
}

// Set various properties of a given widget
extern "C" void gui_setCaption(MyGUI::WidgetPtr p, char* s)
{
  p->setCaption(s);
}

extern "C" void gui_setNeedMouseFocus(MyGUI::WidgetPtr p, int32_t b)
{ p->setNeedMouseFocus(b); }

extern "C" void gui_setTextColor(MyGUI::WidgetPtr p, float r,float g,float b)
{
  MyGUI::StaticText *st = dynamic_cast<MyGUI::StaticText*>(p);
  if(st != NULL)
    st->setTextColour(Ogre::ColourValue(b,g,r));
}

extern "C" void gui_setCoord(MyGUI::WidgetPtr p,
                             int32_t x,int32_t y,int32_t w,int32_t h)
{ p->setCoord(x,y,w,h); }

// Various ways to get or create widgets
extern "C" MyGUI::WidgetPtr gui_loadLayout(char *file, char *prefix,
                                           MyGUI::WidgetPtr parent)
{
  // Get the list of Widgets in this layout
  MyGUI::VectorWidgetPtr wlist;
  wlist = MyGUI::LayoutManager::getInstance().
    loadLayout(file, prefix, parent);

  MyGUI::VectorWidgetPtr::iterator iter;
  iter = wlist.begin();

  // Return null if the list is empty
  if(wlist.end() == iter)
    return NULL;

  MyGUI::WidgetPtr res = *iter;

  ++iter;

  if(iter != wlist.end())
    std::cout << "WARNING: Layout '" << file
              << "' has more than one root widget. Ignored.\n";

  return res;
}

extern "C" MyGUI::WidgetPtr gui_getChild(MyGUI::WidgetPtr p, char* name)
{
  return p->findWidget(name);
}

extern "C" MyGUI::WidgetPtr gui_createText(const char *skin,
                                           int32_t x, int32_t y,
                                           int32_t w, int32_t h,
                                           const char *layer)
{
  return mGUI->createWidget<MyGUI::StaticText>
    (skin,
     x,y,w,h,
     MyGUI::ALIGN_LEFT | MyGUI::ALIGN_TOP,
     layer);
}

// Copied from MyGUI demo code, with a few modifications
class Layout
{
public:
  Layout(const std::string & _layout, MyGUI::WidgetPtr _parent = nullptr)
    : mMainWidget(nullptr)
  {
    initialise(_layout, _parent);
  }

  template <typename T>
  void getWidget(T * & _widget, const std::string & _name, bool _throw = true)
  {
    _widget = nullptr;
    for (MyGUI::VectorWidgetPtr::iterator iter=mListWindowRoot.begin();
         iter!=mListWindowRoot.end(); ++iter)
      {
        MyGUI::WidgetPtr find = (*iter)->findWidget(mPrefix + _name);
        if (nullptr != find)
          {
            T * cast = find->castType<T>(false);
            if (nullptr != cast)
              _widget = cast;
            else if (_throw)
              {
                MYGUI_EXCEPT("Error cast : dest type = '" << T::getClassTypeName()
                       << "' source name = '" << find->getName()
                       << "' source type = '" << find->getTypeName() << "' in layout '" << mLayoutName << "'");
              }
            return;
          }
      }
    MYGUI_ASSERT( ! _throw, "widget name '" << _name << "' in layout '" << mLayoutName << "' not found.");
  }

  void initialise(const std::string & _layout,
                  MyGUI::WidgetPtr _parent = nullptr)
  {
    const std::string MAIN_WINDOW = "_Main";
    mLayoutName = _layout;

    if (mLayoutName.empty())
      mMainWidget = _parent;
    else
      {
        mPrefix = MyGUI::utility::toString(this, "_");
        mListWindowRoot = MyGUI::LayoutManager::getInstance().loadLayout(mLayoutName, mPrefix, _parent);

        const std::string main_name = mPrefix + MAIN_WINDOW;
        for (MyGUI::VectorWidgetPtr::iterator iter=mListWindowRoot.begin(); iter!=mListWindowRoot.end(); ++iter)
          {
            if ((*iter)->getName() == main_name)
              {
                mMainWidget = (*iter);
                break;
              }
          }
        MYGUI_ASSERT(mMainWidget, "root widget name '" << MAIN_WINDOW << "' in layout '" << mLayoutName << "' not found.");
      }
  }

  void shutdown()
  {
    for (VectorBasePtr::iterator iter=mListBase.begin(); iter!=mListBase.end(); ++iter) {
      delete (*iter);
    }
    mListBase.clear();

    MyGUI::LayoutManager::getInstance().unloadLayout(mListWindowRoot);
    mListWindowRoot.clear();
  }

  void setCoord(int x, int y, int w, int h)
  {
    mMainWidget->setCoord(x,y,w,h);
  }

  void setVisible(bool b)
  {
    mMainWidget->setVisible(b);
  }

  virtual ~Layout()
  {
    shutdown();
  }

  void setText(const char* name, const char* caption)
  {
    MyGUI::WidgetPtr pt;
    getWidget(pt, name);
    pt->setCaption(caption);
  }

  void setColor(const char* name, float r, float g, float b)
  {
    MyGUI::WidgetPtr pt;
    getWidget(pt, name);
    gui_setTextColor(pt,r,g,b);
  }

  void setImage(const char* name, const char* imgName)
  {
    MyGUI::StaticImagePtr pt;
    getWidget(pt, name);
    pt->setImageTexture(imgName);
  }

protected:

  MyGUI::WidgetPtr mMainWidget;

  std::string mPrefix;
  std::string mLayoutName;
  MyGUI::VectorWidgetPtr mListWindowRoot;
  typedef std::vector<Layout*> VectorBasePtr;
  VectorBasePtr mListBase;
};

class HUD : public Layout
{
public:
  HUD()
    : Layout("openmw_hud_layout.xml")
  {
    setCoord(0,0,
             mWindow->getWidth(),
             mWindow->getHeight());

    // Energy bars
    getWidget(health, "Health");
    getWidget(magicka, "Magicka");
    getWidget(stamina, "Stamina");

    // Item and spell images and status bars
    getWidget(weapImage, "WeapImage");
    getWidget(weapStatus, "WeapStatus");
    getWidget(spellImage, "SpellImage");
    getWidget(spellStatus, "SpellStatus");

    getWidget(effectBox, "EffectBox");
    getWidget(effect1, "Effect1");

    getWidget(minimap, "MiniMap");
    getWidget(compass, "Compass");

    compass->setImageTexture("compass.dds");
  }

  void setStats(int h, int hmax, int m, int mmax, int s, int smax)
  {
    health->setProgressRange(hmax);
    health->setProgressPosition(h);
    magicka->setProgressRange(mmax);
    magicka->setProgressPosition(m);
    stamina->setProgressRange(smax);
    stamina->setProgressPosition(s);
  }

  void setWeapIcon(const char *str)
  { weapImage->setImageTexture(str); }
  void setSpellIcon(const char *str)
  { spellImage->setImageTexture(str); }

  void setWeapStatus(int s, int smax)
  {
    weapStatus->setProgressRange(smax);
    weapStatus->setProgressPosition(s);
  }
  void setSpellStatus(int s, int smax)
  {
    spellStatus->setProgressRange(smax);
    spellStatus->setProgressPosition(s);
  }

  void setEffect(const char *img)
  { effect1->setImageTexture(img); }

  MyGUI::ProgressPtr health, magicka, stamina;

  MyGUI::StaticImagePtr weapImage, spellImage;
  MyGUI::ProgressPtr weapStatus, spellStatus;

  MyGUI::WidgetPtr effectBox;
  MyGUI::StaticImagePtr effect1;

  MyGUI::StaticImagePtr minimap;
  MyGUI::StaticImagePtr compass;
};

class MapWindow : public Layout
{
public:
  MapWindow()
    : Layout("openmw_map_window_layout.xml")
  {
    setCoord(500,0,320,300);
    mMainWidget->setCaption(cellName);
    setText("WorldButton", "World");
    setImage("Compass", "compass.dds");
  }
};

class MainMenu : public Layout
{
public:
  MainMenu()
    : Layout("openmw_mainmenu_layout.xml")
  {
    setCoord(0,0,
             mWindow->getWidth(),
             mWindow->getHeight());
  }
};

class StatsWindow : public Layout
{
public:
  void setBar(const char* name, const char* tname, int val, int max)
  {
    MyGUI::ProgressPtr pt;
    getWidget(pt, name);
    pt->setProgressRange(max);
    pt->setProgressPosition(val);

    std::stringstream out;
    out << val << "/" << max;
    setText(tname, out.str().c_str());
  }

  StatsWindow()
    : Layout("openmw_stats_window_layout.xml")
  {
    setCoord(0,0,498, 342);
    mMainWidget->setCaption("Playername");

    setText("Health_str", "Health");
    setText("Magicka_str", "Magicka");
    setText("Fatigue_str", "Fatigue");

    setText("Level_str", "Level");
    setText("Race_str", "Race");
    setText("Class_str", "Class");

    setText("LevelText", "5");
    setText("RaceText", "Wood Elf");
    setText("ClassText", "Pilgrim");

    setBar("HBar", "HBarT", 60, 100);
    setBar("MBar", "MBarT", 30, 100);
    setBar("FBar", "FBarT", 80, 100);

    setText("Attrib1", "Strength");
    setText("Attrib2", "Intelligence");
    setText("Attrib3", "Willpower");
    setText("Attrib4", "Agility");
    setText("Attrib5", "Speed");
    setText("Attrib6", "Endurance");
    setText("Attrib7", "Personality");
    setText("Attrib8", "Luck");

    setText("AttribVal1", "30");
    setText("AttribVal2", "40");
    setText("AttribVal3", "30");
    setText("AttribVal4", "75");
    setText("AttribVal5", "50");
    setText("AttribVal6", "40");
    setText("AttribVal7", "50");
    setText("AttribVal8", "40");
  }
};

HUD *hud;
StatsWindow *stats;
MapWindow *map;
MyGUI::WidgetPtr FPSText;
OIS::MouseState state;
bool consoleMode = false;
bool inventoryMode = false;

void enterGui()
{
  guiMode++;

  if(guiMode == 1)
    {
      // If we just entered GUI mode, enable the pointer
      mGUI->showPointer();

      // Restore the GUI mouse position. This is a hack because silly
      // OIS doesn't allow us to set the mouse position ourselves.
      *((OIS::MouseState*)&(mMouse->getMouseState())) = state;
      mGUI->injectMouseMove(state.X.abs, state.Y.abs, 0);
    }
}

void leaveGui()
{
  guiMode--;

  if(guiMode < 0)
    {
      std::cout << "WARNING: guiMode is " << guiMode << "\n";
      guiMode = 0;
    }

  // Are we done with all GUI windows?
  if(guiMode == 0)
    {
      // If so, hide the pointer and store the mouse state for later.
      mGUI->hidePointer();
      state = mMouse->getMouseState();
    }
}

#include "cpp_console.cpp"

extern "C" void gui_toggleGui()
{
  if(inventoryMode)
    {
      leaveGui();
      if(stats)
        stats->setVisible(false);
      if(map)
        map->setVisible(false);
    }
  else
    {
      enterGui();
      if(stats)
        stats->setVisible(true);
      if(map)
        map->setVisible(true);
    }

  inventoryMode = !inventoryMode;
}

extern "C" int32_t* gui_getGuiModePtr() { return &guiMode; }

extern "C" void gui_showHUD()
{
  if(hud)
    hud->setVisible(true);
}

extern "C" void gui_setupGUI(int32_t debugOut)
{
  ResourceGroupManager::getSingleton().
    addResourceLocation("media_mygui", "FileSystem", "General");

  // Enable/disable logging to stdout
  MyGUI::LogManager::initialise();
  MyGUI::LogManager::setSTDOutputEnabled(debugOut);

  mGUI = new MyGUI::Gui();
  mGUI->initialise(mWindow);

  int mWidth = mWindow->getWidth();
  int mHeight = mWindow->getHeight();

  stats = new StatsWindow();
  map = new MapWindow();
  cons = new Console();

  // Hide all the windows at startup
  stats->setVisible(false);
  map->setVisible(false);
  cons->setVisible(false);
  guiMode = 1;
  leaveGui();

  // Start the mouse in the middle of the screen
  state.X.abs = mWidth / 2;
  state.Y.abs = mHeight / 2;

  // Set up the HUD
  hud = new HUD();

  hud->setStats(60, 100,
                30, 100,
                80, 100);

  hud->setWeapIcon("icons\\w\\tx_knife_iron.dds");
  hud->setWeapStatus(90, 100);
  hud->setSpellIcon("icons\\s\\b_tx_s_rstor_health.dds");
  hud->setSpellStatus(65, 100);

  hud->setEffect("icons\\s\\tx_s_chameleon.dds");

  hud->setVisible(false);

  //new MainMenu();
}