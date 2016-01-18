#ifndef _PARAMETERS_PANEL_
#define _PARAMETERS_PANEL_

#include <string>
#include <map>

#include <QWidget>
#include <QHBoxLayout>

class ParameterWidget;

class ParametersPanel : public QWidget {
  //  Q_OBJECT

  public:
    ParametersPanel(QWidget * parent_);

    void addParameter(std::string const & parameterName);
    // concenient way to retrieve the parameter value
    int operator[](std::string const & parameterName);
  protected:
    std::map< std::string, ParameterWidget *>  parametersMap;
    QVBoxLayout * verticalLayout;
};

#endif // _PARAMETERS_PANEL_

