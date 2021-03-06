// -*- mode: c++; c-file-style: "linux"; c-basic-offset: 2; indent-tabs-mode: nil -*-
//
//  Copyright (C) 2014-2018 Gunter Königsmann <wxMaxima@physikbuch.de>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
//  SPDX-License-Identifier: GPL-2.0+

/*! \file
  This file defines the class ConjugateCell

  ConjugateCell is the Cell type that represents the field that represents the 
  conjugate() command.
 */

#include "ConjugateCell.h"
#include "CellImpl.h"
#include "VisiblyInvalidCell.h"

ConjugateCell::ConjugateCell(GroupCell *parent, Configuration **config) :
    Cell(parent, config),
    m_innerCell(std::make_unique<VisiblyInvalidCell>(parent,config)),
    m_open(std::make_unique<TextCell>(parent, config, "conjugate(")),
    m_close(std::make_unique<TextCell>(parent, config, ")"))
{
  InitBitFields();
  static_cast<TextCell&>(*m_open).DontEscapeOpeningParenthesis();
}

// Old cppcheck bugs:
// cppcheck-suppress uninitMemberVar symbolName=ConjugateCell::m_open
// cppcheck-suppress uninitMemberVar symbolName=ConjugateCell::m_close
ConjugateCell::ConjugateCell(const ConjugateCell &cell):
 ConjugateCell(cell.m_group, cell.m_configuration)
{
  CopyCommonData(cell);
  if (cell.m_innerCell)
    SetInner(cell.m_innerCell->CopyList());
}

DEFINE_CELL(ConjugateCell)

void ConjugateCell::SetInner(std::unique_ptr<Cell> &&inner)
{
  if (!inner)
    return;
  m_innerCell = std::move(inner);
}

void ConjugateCell::Recalculate(AFontSize fontsize)
{
  if(!NeedsRecalculation(fontsize))
    return;

  m_innerCell->RecalculateList(fontsize);
  m_open->RecalculateList(fontsize);
  m_close->RecalculateList(fontsize);

  if(!m_isBrokenIntoLines)
  {
    m_width = m_innerCell->GetFullWidth() + Scale_Px(8);
    m_height = m_innerCell->GetHeightList() + Scale_Px(6);
    m_center = m_innerCell->GetCenterList() + Scale_Px(6);
  }
  else
  {
    // The ConjugateCell itself isn't displayed if it is broken into lines.
    // insted m_open, m_innerCell and m_close are => We can set our size to 0
    // in this case.
    m_width = 0;
    m_height = 0;
    m_center = 0;
  }
  Cell::Recalculate(fontsize);
}

void ConjugateCell::Draw(wxPoint point)
{
  Cell::Draw(point);
  if (DrawThisCell(point))
  {
    Configuration *configuration = (*m_configuration);
    
    wxDC *dc = configuration->GetDC();
    SetPen();
    wxPoint in;
    in.x = point.x + Scale_Px(4);
    in.y = point.y;
    m_innerCell->DrawList(in);

    dc->DrawLine(point.x + Scale_Px(2),
                 point.y - m_center + Scale_Px(6),
                 point.x + m_width - Scale_Px(2) - 1,
                 point.y - m_center + Scale_Px(6)
      );
    //                point.y - m_center + m_height - Scale_Px(2));
  }
}

wxString ConjugateCell::ToString() const
{
  if (m_isBrokenIntoLines)
    return wxEmptyString;
  else
    return wxT("conjugate(") + m_innerCell->ListToString() + wxT(")");
}

wxString ConjugateCell::ToMatlab() const
{
  if (m_isBrokenIntoLines)
	return wxEmptyString;
  else
	return wxT("conjugate(") + m_innerCell->ListToMatlab() + wxT(")");
}

wxString ConjugateCell::ToTeX() const
{
  if (m_isBrokenIntoLines)
    return wxEmptyString;
  else
    return wxT("\\overline{") + m_innerCell->ListToTeX() + wxT("}");
}

wxString ConjugateCell::ToMathML() const
{
//  return wxT("<apply><conjugate/><ci>") + m_innerCell->ListToMathML() + wxT("</ci></apply>");
  return wxT("<mover accent=\"true\">") + m_innerCell->ListToMathML() +
         wxT("<mo>&#xaf;</mo></mover>\n");
}

wxString ConjugateCell::ToOMML() const
{
  return wxT("<m:bar><m:barPr><m:pos m:val=\"top\"/> </m:barPr><m:e>") +
         m_innerCell->ListToOMML() + wxT("</m:e></m:bar>");
}

wxString ConjugateCell::ToXML() const
{
  wxString flags;
  if (m_forceBreakLine)
    flags += wxT(" breakline=\"true\"");

  return wxT("<cj") + flags + wxT(">") + m_innerCell->ListToXML() + wxT("</cj>");
}

bool ConjugateCell::BreakUp()
{
  if (!m_isBrokenIntoLines)
  {
    Cell::BreakUp();
    m_isBrokenIntoLines = true;
    m_open->SetNextToDraw(m_innerCell);
    m_innerCell->last()->SetNextToDraw(m_close);
    m_close->SetNextToDraw(m_nextToDraw);
    m_nextToDraw = m_open;
    ResetCellListSizes();
    m_height = 0;
    m_center = 0;
    return true;
  }
  return false;
}

void ConjugateCell::SetNextToDraw(Cell *next)
{
  if(m_isBrokenIntoLines)
    m_close->SetNextToDraw(next);
  else
    m_nextToDraw = next;
}
