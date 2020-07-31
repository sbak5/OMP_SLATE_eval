//////////////////////////////////////////////////////////////////
// (c) Copyright 2008-  by Jeongnim Kim
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//   National Center for Supercomputing Applications &
//   Materials Computation Center
//   University of Illinois, Urbana-Champaign
//   Urbana, IL 61801
//   e-mail: jnkim@ncsa.uiuc.edu
//
// Supported by
//   National Center for Supercomputing Applications, UIUC
//   Materials Computation Center, UIUC
//////////////////////////////////////////////////////////////////
// -*- C++ -*-
/**@file multidet.cpp
 * @brief Test codes for multidets
 */
#include "Utilities/OhmmsInfo.h"
#include "Utilities/RandomGenerator.h"
#include "Utilities/Timer.h"
#include "Message/Communicate.h"
#include "Message/OpenMP.h"
#include <Configuration.h>
#include <OhmmsPETE/OhmmsVector.h>
#include <OhmmsPETE/OhmmsMatrix.h>
#include <QMCWaveFunctions/Spline3D/EinsplineSetTemp.hpp>
#include <algorithm>

using namespace qmcplusplus;

int main(int argc, char** argv)
{
  OHMMS::Controller->initialize(argc,argv);
  OhmmsInfo Welcome(argc,argv,OHMMS::Controller->rank());
  Random.init(0,1,11);
  bspline_engine<multi_UBspline_3d_d,true,true,true> ortho_real_engine;
  bspline_engine<multi_UBspline_3d_d,true,false,true> real_engine;
  bspline_engine<multi_UBspline_3d_z,true,true,false> complex_engine;
  Vector<double> psi_d(4);
  Vector<TinyVector<double,3> > dpsi_d(4);
  Vector<double> d2psi_d(4);
  Vector<complex<double> > psi_z(4);
  Vector<TinyVector<complex<double> ,3> > dpsi_z(4);
  Vector<complex<double> > d2psi_z(4);
  Vector<float> psi_f(4);
  Vector<TinyVector<float,3> > dpsi_f(4);
  Vector<float> d2psi_f(4);
  TinyVector<double,3> pos_d;
  TinyVector<float,3> pos_f;
  ortho_real_engine.evaluate(pos_d,psi_d.data());
  real_engine.evaluate(pos_d,psi_d.data());
  real_engine.evaluate(pos_f,psi_f.data());
  complex_engine.evaluate(pos_d,psi_d.data());
  complex_engine.evaluate(pos_f,psi_f.data());
  complex_engine.evaluate(pos_d,psi_z.data());
  OHMMS::Controller->finalize();
  return 0;
}

/***************************************************************************
 * $RCSfile$   $Author: jnkim $
 * $Revision: 1770 $   $Date: 2007-02-17 17:45:38 -0600 (Sat, 17 Feb 2007) $
 * $Id: OrbitalBase.h 1770 2007-02-17 23:45:38Z jnkim $
 ***************************************************************************/
