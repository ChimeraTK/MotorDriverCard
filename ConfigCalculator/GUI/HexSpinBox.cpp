/*
 * HexSpinBox.cc
 *
 *  Created on: Feb 12, 2015
 *      Author: varghese
 */

#include "HexSpinBox.h"
#include <sstream>

HexSpinBox::HexSpinBox(QWidget* parent_)
    : QDoubleSpinBox(parent_),_validator(NULL) {
  // right now the hex spin box accepts [1,8] characters
  QString regex("[0-9A-Fa-f]{1,8}");
  _validator = new QRegExpValidator(QRegExp(regex), this);
  setDecimals(0);
  setPrefix("0x");
}

HexSpinBox::~HexSpinBox() { delete _validator; }

QString HexSpinBox::textFromValue(double value_) const {
  // QString::number has a bug and gives negative values as 64 bit hex.
  // So we have to do it manually.
  std::stringstream s;
  s << std::hex << static_cast<uint>(value_);
  return QString(s.str().c_str());
}

double HexSpinBox::valueFromText(const QString& text_) const {
  bool conversionStatus = false;
  uint convertedValue = text_.toUInt(&conversionStatus, 16);
  return convertedValue;
}

QValidator::State HexSpinBox::validate(QString& text_, int& pos_) const {
  // TODO: Make the number of hex characters accepted in line with the range
  return (_validator->validate(text_, pos_));
}
