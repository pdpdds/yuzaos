#include "main_window.hpp"
#include "key_event.hpp"
#include "open_dialog.hpp"
#include "save_dialog.hpp"
#include "text_file.hpp"
#include "dialog.hpp"
#include "application.hpp"
#include "full_file_name.hpp"
#include <algorithm.h>
#include <assert.h>
#include <iostream>
#include "functional.hpp"
//#include "eastlbind.h"

void MyFunction(int a)
{
    printf("%d\n", a);
}

MainWindow::MainWindow(Widget *parent):
  Widget(parent),
  activeScreen_(nullptr),
  layout_(Layout::Vertical),
  tabs_(this),
  statusBar_(this),
  screenLayout_(Layout::Vertical)
{
  activeScreen_ = new Screen(this);
  //20201105
  using namespace std::placeholders;
  tabs_.setTextBuffer = std::_bind::bind(&MainWindow::setTextBuffer, this, _1);
  tabs_.deleteTextBuffer = std::_bind::bind(&MainWindow::deleteTextBuffer, this, _1);
  //std::function<void(int)>     func = std::_bind::bind(&MyFunction, _1);
  //func(5);
  //tabs_.setTextBuffer = BIND::bind(&MainWindow::setTextBuffer, BIND::__1, this);
  //tabs_.deleteTextBuffer = BIND::bind(&MainWindow::deleteTextBuffer, BIND::__1, this);

  activeScreen_->setStatusBar(&statusBar_);
  setLayout(&layout_);
  layout_.addLayoutable(&tabs_);
  screenLayout_.addLayoutable(activeScreen_);
  layout_.addLayoutable(&screenLayout_);
  layout_.addLayoutable(&statusBar_);
  activeScreen_->setFocus();
}

bool MainWindow::keyPressEvent(KeyEvent &e)
{
  eastl::cout << __func__ << " key: " << e.key() << " modifiers: " << e.modifiers() << endl;
  bool result1 = true;
  if ((e.modifiers() & (KeyEvent::MLCtrl | KeyEvent::MRCtrl)) != 0)
  {
    switch (e.key())
    {
    case KeyEvent::K2:
      wholeScreen();
      break;
    case KeyEvent::K3:
      split(Layout::Vertical);
      break;
    case KeyEvent::K4:
      split(Layout::Horizontal);
      break;
    case KeyEvent::KO:
      {
        auto &buffersList = tabs_.textBuffersList();
        auto tmp = eastl::find_if(eastl::begin(buffersList), eastl::end(buffersList),
                                [](BaseTextBuffer *x) 
                                { 
                                    
                                    return x->As<OpenDialog>();
                                  //return dynamic_cast<OpenDialog *>(x); 
                                });
        if (tmp == end(buffersList))
        {
          auto openDialog = new OpenDialog(activeScreen_);
          //20201105
          /*using namespace std::placeholders;
          openDialog->openFile = std::bind(&MainWindow::openFile, this, _1, _2);
          */
          tabs_.addTextBuffer(openDialog);
        }
        else
          tabs_.setActiveTextBuffer(*tmp);
        break;
      }
    case KeyEvent::KS:
      save();
      break;
    case KeyEvent::KN:
      tabs_.addTextBuffer(new TextFile);
      break;
    case KeyEvent::KW:
      if ((tabs_.activeTextBuffer()->As<TextFile>()) && tabs_.activeTextBuffer()->isModified())
      {
        if (!statusBar_.textBuffer())
        {
          auto d = new Dialog(L"The file is modified. Do you want to save it before closing?");
          statusBar_.setTextBuffer(d);
          //20201105
          //using namespace std::placeholders;
          //d->result = std::bind(&MainWindow::closeActiveTextBuffer, this, _1);
        }
      }
      else
        tabs_.closeActiveTextBuffer();
      break;
    case KeyEvent::KPageUp:
    case KeyEvent::KLeft:
      tabs_.switchToPrevTextBuffer();
      break;
    case KeyEvent::KPageDown:
    case KeyEvent::KRight:
      tabs_.switchToNextTextBuffer();
      break;
    case KeyEvent::KTab:
      switchToNextScreen();
      break;
    default:
      result1 = false;
    }
  }
  else if ((e.modifiers() & (KeyEvent::MLAlt | KeyEvent::MRAlt)) != 0)
  {
    switch (e.key())
    {
    case KeyEvent::KLeft:
      switchToPrevScreen();
      break;
    case KeyEvent::KRight:
      switchToNextScreen();
      break;
    default:
      result1 = false;
    }
  }
  else
    result1 = false;

  bool result2 = true;
  if ((e.modifiers() & (KeyEvent::MCtrl | KeyEvent::MShift)) != 0)
  {
    switch (e.key())
    {
    case KeyEvent::KLeft:
      tabs_.moveTextBufferLeft();
      break;
    case KeyEvent::KRight:
      tabs_.moveTextBufferRight();
      break;
    default:
      result2 = false;
    }
  }
  else
    result2 = false;

  return result1 || result2;
}

void MainWindow::openFile(OpenDialog *, const eastl::string &fileName)
{
  auto &buffersList = tabs_.textBuffersList();
  auto fullFileName = getFullFileName(fileName);
  auto tmp = eastl::find_if(eastl::begin(buffersList), eastl::end(buffersList),
                          [&fullFileName](BaseTextBuffer *x) 
                          {
                            if (auto textFile = x->As<TextFile>())
                              return textFile->fileName() == fullFileName;
                            else
                              return false;
                          });
  if (tmp == eastl::end(buffersList))
    tabs_.addTextBuffer(new TextFile(fileName));
  else
    tabs_.setActiveTextBuffer(*tmp);
}

void MainWindow::saveAs(SaveDialog *sender, TextFile *textFile, const eastl::string &fileName)
{
  tabs_.closeTextBuffer(sender);
  tabs_.setActiveTextBuffer(textFile);
  textFile->saveAs(fileName);
}

void MainWindow::saveAndClose(SaveDialog *sender, TextFile *textFile, const eastl::string &fileName)
{
  tabs_.closeTextBuffer(sender);
  tabs_.closeTextBuffer(textFile);
  textFile->saveAs(fileName);
}

void MainWindow::closeActiveTextBuffer(Dialog::Answer value)
{
  auto d = statusBar_.textBuffer();
  Application::instance()->queueDelete(d);
  statusBar_.setTextBuffer(nullptr);
  if (auto textFile = activeScreen_->textBuffer()->As<TextFile>())
  {
    switch (value)
    {
    case Dialog::Yes:
      if (textFile->fileName().empty())
      {
        auto saveDialog = new SaveDialog(activeScreen_, textFile);
        tabs_.addTextBuffer(saveDialog);
        activeScreen_->setCursor(0, 1);
        //20201105
        //using namespace std::placeholders;
        //saveDialog->saveAs = std::bind(&MainWindow::saveAndClose, this, _1, _2, _3);
      }
      else
      {
        textFile->save();
        tabs_.closeActiveTextBuffer();                
      }
      break;
    case Dialog::No:
      tabs_.closeActiveTextBuffer();
      break;
    case Dialog::Cancel:
      break;
            
    }
    tabs_.update();
  }
}

void MainWindow::save()
{
  if (auto textFile = activeScreen_->textBuffer()->As<TextFile>())
  {
    if (textFile->fileName().empty())
    {
      auto saveDialog = new SaveDialog(activeScreen_, textFile);
      tabs_.addTextBuffer(saveDialog);
      activeScreen_->setCursor(0, 1);
      //20201105
      //using namespace std::placeholders;
      //saveDialog->saveAs = std::bind(&MainWindow::saveAs, this, _1, _2, _3);
    }
    else
      textFile->save();
    tabs_.update();
  }
}

static void markForDeleteRecursively(Layoutable *value)
{
  if (auto layout = value->As<Layout>())
  {
    auto children = layout->children();
    for (auto child: children)
    {
      markForDeleteRecursively(child);
      child->parentLayout()->removeLayoutable(child);
      Application::instance()->queueDelete(child);
    }
  }
}

void MainWindow::wholeScreen()
{
  markForDeleteRecursively(&screenLayout_);
  auto screen = activeScreen_;
  activeScreen_ = new Screen(this);
  activeScreen_->setStatusBar(&statusBar_);
  screenLayout_.addLayoutable(activeScreen_);
  activeScreen_->setTextBuffer(screen->textBuffer());
  activeScreen_->setFocus();
}

void MainWindow::split(Layout::Style style)
{
  auto layout = activeScreen_->parentLayout();
  layout->setStyle(style);
  auto l1 = new Layout(Layout::Vertical);
  auto l2 = new Layout(Layout::Vertical);
  layout->removeLayoutable(activeScreen_);
  layout->addLayoutable(l1);
  layout->addLayoutable(l2);
  l1->addLayoutable(activeScreen_);
  auto s2 = new Screen(this);
  s2->setStatusBar(&statusBar_);
  s2->setTextBuffer(activeScreen_->textBuffer());
  l2->addLayoutable(s2);
}

static eastl::vector<Screen *> getListOfScreens(Layoutable *l)
{
    eastl::vector<Screen *> res;
  if (auto screen = l->As<Screen>())
    res.push_back(screen);
  else if (auto layout = l->As<Layout>())
  {
    auto children = layout->children();
    for (Layoutable *child: children)
    {
      eastl::vector<Screen *> tmp = getListOfScreens(child);
      res.insert(eastl::end(res), eastl::begin(tmp), eastl::end(tmp));
    }
  }
  return res;
}

void MainWindow::switchToPrevScreen()
{
    eastl::vector<Screen *> list = getListOfScreens(&screenLayout_);
  auto iter = eastl::find(begin(list), eastl::end(list), activeScreen_);
  assert(iter != end(list));
  if (iter != eastl::begin(list))
    --iter;
  else
    iter = eastl::end(list) - 1;
  activeScreen_ = *iter;
  tabs_.setActiveTextBuffer(activeScreen_->textBuffer());
  activeScreen_->setFocus();
}

void MainWindow::switchToNextScreen()
{
    eastl::vector<Screen *> list = getListOfScreens(&screenLayout_);
  auto iter = eastl::find(begin(list), eastl::end(list), activeScreen_);
  assert(iter != end(list));
  if (iter + 1 != eastl::end(list))
    ++iter;
  else
    iter = begin(list);
  activeScreen_ = *iter;
  tabs_.setActiveTextBuffer(activeScreen_->textBuffer());
  activeScreen_->setFocus();
}

void MainWindow::setTextBuffer(BaseTextBuffer *textBuffer)
{
  activeScreen_->setTextBuffer(textBuffer);
}

static void internalDeleteTextBuffer(Layoutable *l, BaseTextBuffer *textBuffer)
{
  if (auto screen = l->As<Screen>())
  {
    if (screen->textBuffer() == textBuffer)
      screen->setTextBuffer(nullptr);
  }
  else if (auto layout = l->As<Layout>())
  {
    auto children = layout->children();
    for (Layoutable *child: children)
      internalDeleteTextBuffer(child, textBuffer);
  }
}


void MainWindow::deleteTextBuffer(BaseTextBuffer *textBuffer)
{
  internalDeleteTextBuffer(&screenLayout_, textBuffer);
}
