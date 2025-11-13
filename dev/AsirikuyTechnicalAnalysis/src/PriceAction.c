/**
 * @file
 * @brief     Price action calculations including Asirikuy specific, non-standard, and standard methods which are not available in TaLib.
 * 
 * @author    Morgan Doel (Initial implementation)
 * @author    Daniel Fernandez (Assisted with design and code styling)
 * @author    Maxim Feinshtein (Assisted with design and code styling)
 * @version   F4.x.x
 * @date      2012
 *
 * @copyright END-USER LICENSE AGREEMENT FOR ASIRIKUY SOFTWARE. IMPORTANT PLEASE READ THE TERMS AND CONDITIONS OF THIS LICENSE AGREEMENT CAREFULLY BEFORE USING THIS SOFTWARE: 
 * @copyright Asirikuy's End-User License Agreement ("EULA") is a legal agreement between you (either an individual or a single entity) and Asirikuy for the use of the Asirikuy Framework in both source and binary forms. By installing, copying, or otherwise using the Asirikuy Framework, you agree to be bound by the terms of this EULA. This license agreement represents the entire agreement concerning the program between you and Asirikuy, (referred to as "licenser"), and it supersedes any prior proposal, representation, or understanding between the parties. If you do not agree to the terms of this EULA, do not install or use the Asirikuy Framework.
 * @copyright The Asirikuy Framework is protected by copyright laws and international copyright treaties, as well as other intellectual property laws and treaties. The Asirikuy Framework is licensed, not sold.
 * @copyright 1. GRANT OF LICENSE.
 * @copyright The Asirikuy Framework is licensed as follows:
 * @copyright (a) Installation and Use.
 * @copyright Asirikuy grants you the right to install and use copies of the Asirikuy Framework in both source and binary forms for personal and business use. You may also make modifications to the source code.
 * @copyright (b) Backup Copies.
 * @copyright You may make copies of the Asirikuy Framework as may be necessary for backup and archival purposes.
 * @copyright 2. DESCRIPTION OF OTHER RIGHTS AND LIMITATIONS.
 * @copyright (a) Maintenance of Copyright Notices.
 * @copyright You must not remove or alter any copyright notices on any and all copies of the Asirikuy Framework.
 * @copyright (b) Distribution.
 * @copyright You may not distribute copies of the Asirikuy Framework in binary or source forms to third parties outside of the Asirikuy community.
 * @copyright (c) Rental.
 * @copyright You may not rent, lease, or lend the Asirikuy Framework.
 * @copyright (d) Compliance with Applicable Laws.
 * @copyright You must comply with all applicable laws regarding use of the Asirikuy Framework.
 * @copyright 3. TERMINATION
 * @copyright Without prejudice to any other rights, Asirikuy may terminate this EULA if you fail to comply with the terms and conditions of this EULA. In such event, you must destroy all copies of the Asirikuy Framework in your possession.
 * @copyright 4. COPYRIGHT
 * @copyright All title, including but not limited to copyrights, in and to the Asirikuy Framework and any copies thereof are owned by Asirikuy or its suppliers. All title and intellectual property rights in and to the content which may be accessed through use of the Asirikuy Framework is the property of the respective content owner and may be protected by applicable copyright or other intellectual property laws and treaties. This EULA grants you no rights to use such content. All rights not expressly granted are reserved by Asirikuy.
 * @copyright 5. NO WARRANTIES
 * @copyright Asirikuy expressly disclaims any warranty for the Asirikuy Framework. The Asirikuy Framework is provided 'As Is' without any express or implied warranty of any kind, including but not limited to any warranties of merchantability, noninfringement, or fitness of a particular purpose. Asirikuy does not warrant or assume responsibility for the accuracy or completeness of any information, text, graphics, links or other items contained within the Asirikuy Framework. Asirikuy makes no warranties respecting any harm that may be caused by the transmission of a computer virus, worm, time bomb, logic bomb, or other such computer program. Asirikuy further expressly disclaims any warranty or representation to Authorized Users or to any third party.
 * @copyright 6. LIMITATION OF LIABILITY
 * @copyright In no event shall Asirikuy or any contributors to the Asirikuy Framework be liable for any damages (including, without limitation, lost profits, business interruption, or lost information) rising out of 'Authorized Users' use of or inability to use the Asirikuy Framework, even if Asirikuy has been advised of the possibility of such damages. In no event will Asirikuy or any contributors to the Asirikuy Framework be liable for loss of data or for indirect, special, incidental, consequential (including lost profit), or other damages based in contract, tort or otherwise. Asirikuy and contributors to the Asirikuy Framework shall have no liability with respect to the content of the Asirikuy Framework or any part thereof, including but not limited to errors or omissions contained therein, libel, infringements of rights of publicity, privacy, trademark rights, business interruption, personal injury, loss of privacy, moral rights or the disclosure of confidential information.
 */

#include "Precompiled.h"
#include "PriceAction.h"

typedef enum strategy_t
{
  SIGNAL_NONE = 0,
  SIGNAL_OPEN_TRADE = 1,
  INF = 999999
} StrategyActions;

AsirikuyReturnCode minMaxIndex(const double* pPrice, int arraySize, int period, int shift, int* pOutMinIndex, int* pOutMaxIndex)
{
  int i;

  if(pPrice == NULL)
  {
    fprintf(stderr, "[CRITICAL] minMaxIndex() failed. pPrice = NULL\n");
    return NULL_POINTER;
  }

  if(pOutMinIndex == NULL)
  {
    fprintf(stderr, "[CRITICAL] minMaxIndex() failed. pOutMinIndex = NULL\n");
    return NULL_POINTER;
  }

  if(pOutMaxIndex == NULL)
  {
    fprintf(stderr, "[CRITICAL] minMaxIndex() failed. pOutMaxIndex = NULL\n");
    return NULL_POINTER;
  }

  *pOutMinIndex = *pOutMaxIndex = arraySize - 1 - period - shift;

  for(i = arraySize -1 - shift; (i > arraySize - 1 - period - shift) && (i >= 0); i--)
  {
    if(pPrice[i] < pPrice[*pOutMinIndex])
    {
      *pOutMinIndex = i;
    }

    if(pPrice[i] > pPrice[*pOutMaxIndex])
    {
      *pOutMaxIndex = i;
    }

    fprintf(stderr, "[DEBUG] minMaxIndex() arraySize = %d, period = %d, shift = %d, Index %d = %f", arraySize, period, shift, i, pPrice[i]);
  }

  return SUCCESS;
}

AsirikuyReturnCode minIndex(const double* pPrice, int arraySize, int period, int shift, int* pOutMinIndex)
{
  int i;

  if(pPrice == NULL)
  {
    fprintf(stderr, "[CRITICAL] minIndex() failed. pPrice = NULL\n");
    return NULL_POINTER;
  }

  if(pOutMinIndex == NULL)
  {
    fprintf(stderr, "[CRITICAL] minIndex() failed. pOutMinIndex = NULL\n");
    return NULL_POINTER;
  }

  *pOutMinIndex = arraySize - 1 - period - shift;

  for(i = arraySize -1 - shift; (i > arraySize - 1 - period - shift) && (i >= 0); i--)
  {
    if(pPrice[i] < pPrice[*pOutMinIndex])
    {
      *pOutMinIndex = i;
    }

    fprintf(stderr, "[DEBUG] minIndex() arraySize = %d, period = %d, shift = %d, Index %d = %f", arraySize, period, shift, i, pPrice[i]);
  }

  return SUCCESS;
}

AsirikuyReturnCode maxIndex(const double* pPrice, int arraySize, int period, int shift, int* pOutMaxIndex)
{
  int i;

  if(pPrice == NULL)
  {
    fprintf(stderr, "[CRITICAL] maxIndex() failed. pPrice = NULL\n");
    return NULL_POINTER;
  }

  if(pOutMaxIndex == NULL)
  {
    fprintf(stderr, "[CRITICAL] maxIndex() failed. pOutMaxIndex = NULL\n");
    return NULL_POINTER;
  }

  *pOutMaxIndex = arraySize - 1 - period - shift;

  for(i = arraySize -1 - shift; (i > arraySize - 1 - period - shift) && (i >= 0); i--)
  {
    if(pPrice[i] > pPrice[*pOutMaxIndex])
    {
      *pOutMaxIndex = i;
    }

    fprintf(stderr, "[DEBUG] maxIndex() arraySize = %d, period = %d, shift = %d, Index %d = %f", arraySize, period, shift, i, pPrice[i]);
  }

  return SUCCESS;
}

int CheckOpenConditions(int j, double p, double q, double eP, double eM, double C, int isUpStep)
{
   int jP, jM;
   
   if (eP >= eM)  // Strategy 2.1
   {
      // calculate jP, jM
      jP = VHPjPFind (j, p, q, eP, eM, C);
      if (isUpStep == 1)
      {
         if (j >= jP)
         {
            return (SIGNAL_OPEN_TRADE);
         }
      }
      else
      {
         jM = DMjMFind (j, jP, p, q, eP, eM, C);
         if (j >= jM)
         {
            return (SIGNAL_OPEN_TRADE);
         }
      }      
   }
   else               // Strategy 2.2
   {
      // calculate jM, jP
      jM = VHMjMFind (j, p, q, eP, eM, C);
      if (isUpStep == 0)
      {
         if (j >= jM)
         {
            return (SIGNAL_OPEN_TRADE);
         }
      }
      else
      {
         jP = DPjPFind (j, jM, p, q, eP, eM, C);
         if (j >= jP)
         {
            return (SIGNAL_OPEN_TRADE);
         }
      }
   }
   return (SIGNAL_NONE);
}

double DriftCalculate (double p, double q, double eP, double eM)
{
   double d, r;
   r = p + q - 1;
   d = ((1 - q) * eP + (1 - p) * eM) / (1 - r);
   fprintf(stderr, "[INFO] DRIFT = %lf", d);
   return (d);
}

double VHCCalculate (int j, double p, double q, double eP, double eM)
{
//   Print ("VHC.Calculate\n");
   double r, vhc;
   r = p + q - 1;
   vhc = (eP - eM) / (1 - r * r) * (1 - pow (r, j));
   fprintf(stderr, "[INFO] VHC = %lf", vhc);
   return (vhc);
}

double VHPCalculate (int j, double p, double q, double eP, double eM)
{
//   Print ("VHP.Calculate\n");
   double vhp, d;
   d = DriftCalculate (p, q, eP, eM);
   vhp = d * j + (1 - p) * VHCCalculate (j, p, q, eP, eM);
   fprintf(stderr, "[INFO] VHP = %lf", vhp);
   return (vhp);
}

double VHMCalculate (int j, double p, double q, double eP, double eM)
{
//   Print ("VHM.Calculate\n");
   double vhm, d;
   d = DriftCalculate (p, q, eP, eM);
   vhm = d * j - (1 - q) * VHCCalculate (j, p, q, eP, eM);
   fprintf(stderr, "[INFO] VHM = %lf", vhm);
   return (vhm);
}

int VHPjPFind (int j, double p, double q, double eP, double eM, double C)
{
//   Print ("VHP.jP.Find\n");
   double vhp;
   int jP = 1;
   vhp = VHPCalculate (jP, p, q, eP, eM);
   while (vhp < C)
   {
      jP++;
      if (jP > j)
      {
         jP = INF;
         break;
      }
      vhp = VHPCalculate (jP, p, q, eP, eM);
   }
   fprintf(stderr, "[INFO] VH-jP = %d", jP);
   return (jP);
}

int VHMjMFind (int j, double p, double q, double eP, double eM, double C)
{
//   Print ("VHM.jM.Find\n");
   double vhm;
   int jM = 1;
   vhm = VHMCalculate (jM, p, q, eP, eM);
   while (vhm < C)
   {
      jM++;
      if (jM > j)
      {
         jM = INF;
         break;
      }
      vhm = VHMCalculate (jM, p, q, eP, eM);
   }
   fprintf(stderr, "[INFO] VH-jM = %d", jM);
   return (jM);
}

int DMjMFind (int j, int jP, double p, double q, double eP, double eM, double C)
{
//   Print ("DM.jM.Find\n");
   int jM;
   double DM, bM;
   jM = jP;
   bM = eM / (1 - q) + C;
   DM = (VHMCalculate (jP, p, q, eP, eM) - bM) * pow (q, jM - jP) + bM;
   while (DM < C)
   {
      jM++;
      if (jM > j)
      {
         jM = INF;
         break;
      }
      DM = (VHMCalculate (jP, p, q, eP, eM) - bM) * pow (q, jM - jP) + bM;
   }
   fprintf(stderr, "[INFO] DM-jM = %d", jM);
   return (jM);
}

int DPjPFind (int j, int jM, double p, double q, double eP, double eM, double C)
{
   int jP;
   double DP, bP;
   jP = jM;
   bP = eP / (1 - p) + C;
   DP = (VHPCalculate (jM, p, q, eP, eM) - bP) * pow (p, jP - jM) + bP;
   while (DP < C)
   {
      jP++;
      if (jP > j)
      {
         jP = INF;
         break;
      }
      DP = (VHPCalculate (jM, p, q, eP, eM) - bP) * pow (p, jP - jM) + bP;
   }

   fprintf(stderr, "[INFO] D^P-jP = %d", jP);
   return (jP);
}

/* used to round a double to X decimal places */
double Round(double dbVal, int nPlaces /* = 0 */)
{
    const double dbShift = pow(10.0, nPlaces);
    return  floor(dbVal * dbShift + 0.5) / dbShift; 
}
