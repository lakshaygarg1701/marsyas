/****************************************************************************
**
** Copyright (C) 2005-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "MarGridWindow.h"

MarGridWindow::MarGridWindow(QWidget *parent)
  : QWidget(parent)
{
  
  QPushButton *extract  = new QPushButton(tr("Extract"));
  QPushButton *train = new QPushButton(tr("Train"));
  QPushButton *predict = new QPushButton(tr("Predict"));
  
  QWidget *margrid = new MarGrid();
  QGridLayout *gridLayout = new QGridLayout;
  gridLayout->addWidget(extract, 0, 0);
  gridLayout->addWidget(train, 0, 1);
  gridLayout->addWidget(predict, 0, 2);
  gridLayout->addWidget(margrid, 1, 0, 1, 3);

  connect(extract, SIGNAL(clicked()), margrid, SLOT(extract()));
  connect(train, SIGNAL(clicked()), margrid, SLOT(train()));
  connect(predict, SIGNAL(clicked()), margrid, SLOT(predict()));
  


  setLayout(gridLayout);
  
}


  


	
