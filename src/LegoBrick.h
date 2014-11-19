#ifndef LEGO_BRICK_H
#define LEGO_BRICK_H

#include<QSet>

#include "Vector3.h"
#include "LegoDimensions.h"
#include <iostream>

typedef QPair<int, int> BrickSize;
typedef Vector3 Color3;

class LegoBrick
{
public:

  //Default constructor in order to use the QTL datatypes
  LegoBrick()
    :level_(0), posX_(0), posY_(0), sizeX_(0), sizeY_(0)
  {
    computeHash();
    isOuter_ = false;
    colorId_ = 0;
  }

  LegoBrick(int level, int posX, int posY, int sizeX, int sizeY)
    :level_(level), posX_(posX), posY_(posY), sizeX_(sizeX), sizeY_(sizeY), randColor_(Color3(0, rand()/double(RAND_MAX), rand()/double(RAND_MAX)))
  {
    computeHash();
    isOuter_ = false;
    colorId_ = 0;
  }

  inline int getLevel() const {return level_;}
  inline int getPosX() const {return posX_;}
  inline int getPosY() const {return posY_;}

  inline int getSizeX() const {return sizeX_;}
  inline int getSizeY() const {return sizeY_;}
  inline BrickSize getSize() const {return (sizeX_ <= sizeY_ ? BrickSize(sizeX_, sizeY_) : BrickSize(sizeY_, sizeX_));}
  inline int getKnobNumber() const {return sizeX_*sizeY_;}

  inline const Color3 getRandColor() const {return randColor_;}
  inline int getColorId() const {return colorId_;}
  inline void setColorId(int id) { colorId_ = id;}
  inline void setRandColor(double r, double g, double b) {randColor_[0] = r; randColor_[1] = g; randColor_[2] = b;}


  inline bool isOuter() const {return isOuter_;}
  inline void setIsOuter(bool isOuter) {isOuter_ = isOuter;}

  //We ignore color
  inline bool operator==(const LegoBrick& other) const {
    return (level_ == other.getLevel() &&
            posX_ == other.getPosX() &&
            posY_ == other.getPosY() &&
            sizeX_ == other.getSizeX() &&
            sizeY_ == other.getSizeY()
            );}



  inline uint getHash() const { return hash_ ;}

  inline void computeHash()
  {
    hash_ = 1;
    hash_ = hash_*31 + getLevel();
    hash_ = hash_*31 + getPosX();
    hash_ = hash_*31 + getPosY();
    hash_ = hash_*31 + getSizeX();
    hash_ = hash_*31 + getSizeY();
  }

  inline void print() const{
    std::cout << "LegoBrick(" << level_ << ", " << posX_ << ", " << posY_ << ", " << sizeX_ << ", " << sizeY_ << ", "<< (isOuter_ ? "Outer" : "Inner") << ")" << std::endl;
  }

private:
  int level_;
  int posX_;
  int posY_;

  int sizeX_;//In knobs
  int sizeY_;//In knobs

  Color3 randColor_;
  int colorId_;//Corresponds to the index in the legalColors_ array of LegoCloud

  bool isOuter_;

  uint hash_;
};

/*
inline uint qHash(const LegoBrick& brick)
{
    return brick.getHash();
}*/

#endif
