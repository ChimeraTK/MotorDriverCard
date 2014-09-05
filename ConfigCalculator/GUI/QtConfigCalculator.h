#ifndef QT_CONFIG_CALCULATOR_H
#define QT_CONFIG_CALCULATOR_H

#include "MotorConfigWidget.h"
#include <QTabWidget>
#include <QPushButton>

class QtConfigCalculator: public QWidget{
   Q_OBJECT

 public:
   QtConfigCalculator(QWidget * parent_ = NULL);

 private:
   QVBoxLayout *verticalLayout;
   QTabWidget *motorTabWidget;
   QPushButton *writeButton;

   std::vector<MotorConfigWidget *> motorConfigWidgets;

 private slots:
   void write();

};

#endif// QT_CONFIG_CALCULATOR_H
