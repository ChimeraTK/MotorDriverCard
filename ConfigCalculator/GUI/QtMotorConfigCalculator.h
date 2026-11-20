// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "MotorConfigWidget.h"
#include "ParametersPanel.h"

#include <QPushButton>
#include <QScrollArea>
#include <QTabWidget>

/** A widget containing two MotorConfigWidget in a TabWidget,
 *  and a write button which write the config to an XML file\
 */
class QtMotorConfigCalculator : public QWidget {
  Q_OBJECT

 public:
  /** The constructor expects no parameters except for the
   *  parent widget.
   */
  explicit QtMotorConfigCalculator(QWidget* parent_ = nullptr);
  QtMotorConfigCalculator(QtMotorConfigCalculator const&) = delete;
  QtMotorConfigCalculator& operator=(QtMotorConfigCalculator const&) = delete;

 private:
  QVBoxLayout* _verticalLayout{nullptr};
  QTabWidget* _motorTabWidget{nullptr};
  QPushButton* _writeButton{nullptr};

  QTabWidget* _expertTabWidget{nullptr};
  std::vector<MotorConfigWidget*> _motorConfigWidgets;
  std::vector<ParametersPanel*> _motorExpertWidgets;
  ParametersPanel* _cardExpertWidget{nullptr};

  // NOLINTNEXTLINE(readability-redundant-access-specifiers) Qt slots are declared with the slots keyword for moc
 private slots:
  // Get the file name via dialogue and perform the writing.
  void write();
  void updateCardExpertPanel(int index);
};
