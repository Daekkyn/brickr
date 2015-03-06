#ifndef QDEBUGSTREAM_H
#define QDEBUGSTREAM_H

#include <iostream>
#include <streambuf>
#include <string>

#include <QFileInfo>

#include <QTextEdit>

// to be able to tee (duplicate) streams

#include <streambuf>

class teebuf: public std::streambuf
{
public:
    // Construct a streambuf which tees output to both input
    // streambufs.
    teebuf(std::streambuf * sb1, std::streambuf * sb2)
        : sb1_(sb1)
        , sb2_(sb2)
    {
    }

    std::streambuf *sb1() { return sb1_; }
    std::streambuf *sb2() { return sb2_; }
private:
    // This tee buffer has no buffer. So every character "overflows"
    // and can be put directly into the teed buffers.
    virtual int overflow(int c)
    {
      int const r1 = sb1_->sputc(c);
      int const r2 = sb2_->sputc(c);
      return r1 == EOF || r2 == EOF ? EOF : c;
    }

    // Sync both teed buffers.
    virtual int sync()
    {
        int const r1 = sb1_->pubsync();
        int const r2 = sb2_->pubsync();
        return r1 == 0 && r2 == 0 ? 0 : -1;
    }

    virtual std::streamsize xsputn(const char *p, std::streamsize n)
    {
      sb1_->sputn(p,n);
      sb2_->sputn(p,n);
      return n;
    }

private:
    std::streambuf * sb1_;
    std::streambuf * sb2_;
};

class QDebugStream : public std::streambuf
{
public:
  QDebugStream(QTextEdit* text_edit)
  {
    log_window = text_edit;
  }


protected:

  //This is called when a std::endl has been inserted into the stream
  virtual int_type overflow(int_type c)
  {
//    if (c == '\n')
//    {
//      log_window->insertPlainText("");
//    }
//    else
//    {
      QString str; str.append(c);
      log_window->moveCursor (QTextCursor::End);
      log_window->insertPlainText(str);
//    }
    return c;
  }

  virtual std::streamsize xsputn(const char *p, std::streamsize n)
  {
    QString str = QString::fromUtf8(p, n);
    if(str.contains("\n")){
      QStringList strSplitted = str.split("\n");

      log_window->moveCursor (QTextCursor::End);
      log_window->insertPlainText(strSplitted.at(0)); //Index 0 is still on the same old line

      for(int i = 1; i < strSplitted.size(); i++){
        log_window->insertPlainText(strSplitted.at(i));
      }
    }else{
      log_window->moveCursor (QTextCursor::End);
      log_window->insertPlainText(str);
    }
    return n;
  }


private:
  QTextEdit* log_window;
};

#endif // QDEBUGSTREAM_H
