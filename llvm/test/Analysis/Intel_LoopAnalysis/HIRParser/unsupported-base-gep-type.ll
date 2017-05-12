; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Verify that we are able to parse the load of %0 successfully even though the pointer operand of base GEP %int_cst13.i is unsupported. This is done by restricting parser from tracing back to the unsupported type.

; CHECK: + DO i1 = 0, 49, 1   <DO_LOOP>
; CHECK: |   if (undef #UNDEF# undef)
; CHECK: |   {
; CHECK: |      %1 = (i64*)(%int_cst13.i)[2];
; CHECK: |   }
; CHECK: |   if (undef #UNDEF# undef)
; CHECK: |   {
; CHECK: |   }
; CHECK: + END LOOP


; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "bugpoint-output-6d98849.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%union.tree_node.1651.1740 = type { %struct.tree_function_decl.1650.1739 }
%struct.tree_function_decl.1650.1739 = type { %struct.tree_decl_non_common.1620.1709, %struct.function.1649.1738*, %union.tree_node.1651.1740*, %union.tree_node.1651.1740*, %union.tree_node.1651.1740*, i32 }
%struct.tree_decl_non_common.1620.1709 = type { %struct.tree_decl_with_vis.1619.1708, %union.tree_node.1651.1740*, %union.tree_node.1651.1740*, %union.tree_node.1651.1740*, %union.tree_node.1651.1740* }
%struct.tree_decl_with_vis.1619.1708 = type { %struct.tree_decl_with_rtl.1618.1707, %union.tree_node.1651.1740*, %union.tree_node.1651.1740*, %union.tree_node.1651.1740*, i24 }
%struct.tree_decl_with_rtl.1618.1707 = type { %struct.tree_decl_common.1609.1698, %struct.rtx_def.1617.1706* }
%struct.tree_decl_common.1609.1698 = type { %struct.tree_decl_minimal.1607.1696, %union.tree_node.1651.1740*, i40, i32, %union.tree_node.1651.1740*, %union.tree_node.1651.1740*, %union.tree_node.1651.1740*, %union.tree_node.1651.1740*, %struct.lang_decl.1608.1697* }
%struct.tree_decl_minimal.1607.1696 = type { %struct.tree_common.1606.1695, i32, i32, %union.tree_node.1651.1740*, %union.tree_node.1651.1740* }
%struct.tree_common.1606.1695 = type { %struct.tree_base.1605.1694, %union.tree_node.1651.1740*, %union.tree_node.1651.1740* }
%struct.tree_base.1605.1694 = type { i64 }
%struct.lang_decl.1608.1697 = type opaque
%struct.rtx_def.1617.1706 = type { i32, %union.u.1616.1705 }
%union.u.1616.1705 = type { %struct.block_symbol.1615.1704 }
%struct.block_symbol.1615.1704 = type { [3 x %union.rtunion_def.1610.1699], %struct.object_block.1614.1703*, i64 }
%union.rtunion_def.1610.1699 = type { i8* }
%struct.object_block.1614.1703 = type { %union.section.1611.1700*, i32, i64, %struct.VEC_rtx_gc.1613.1702*, %struct.VEC_rtx_gc.1613.1702* }
%union.section.1611.1700 = type opaque
%struct.VEC_rtx_gc.1613.1702 = type { %struct.VEC_rtx_base.1612.1701 }
%struct.VEC_rtx_base.1612.1701 = type { i32, i32, [1 x %struct.rtx_def.1617.1706*] }
%struct.function.1649.1738 = type { %struct.eh_status.1621.1710*, %struct.control_flow_graph.1641.1730*, %struct.gimple_seq_d.1629.1718*, %struct.gimple_df.1642.1731*, %struct.loops.1643.1732*, %struct.htab.1644.1733*, %union.tree_node.1651.1740*, %union.tree_node.1651.1740*, %union.tree_node.1651.1740*, %union.tree_node.1651.1740*, %struct.machine_function.1647.1736*, %struct.language_function.1648.1737*, %struct.htab.1644.1733*, i32, i32, i32, i32, i32, i32, i8*, i32 }
%struct.eh_status.1621.1710 = type opaque
%struct.control_flow_graph.1641.1730 = type { %struct.basic_block_def.1638.1727*, %struct.basic_block_def.1638.1727*, %struct.VEC_basic_block_gc.1640.1729*, i32, i32, i32, %struct.VEC_basic_block_gc.1640.1729*, i32, [2 x i32], [2 x i32], i32, i32 }
%struct.basic_block_def.1638.1727 = type { %struct.VEC_edge_gc.1633.1722*, %struct.VEC_edge_gc.1633.1722*, i8*, %struct.loop.1634.1723*, [2 x %struct.et_node.1635.1724*], %struct.basic_block_def.1638.1727*, %struct.basic_block_def.1638.1727*, %union.basic_block_il_dependent.1637.1726, i64, i32, i32, i32, i32, i32 }
%struct.VEC_edge_gc.1633.1722 = type { %struct.VEC_edge_base.1632.1721 }
%struct.VEC_edge_base.1632.1721 = type { i32, i32, [1 x %struct.edge_def.1631.1720*] }
%struct.edge_def.1631.1720 = type { %struct.basic_block_def.1638.1727*, %struct.basic_block_def.1638.1727*, %union.edge_def_insns.1630.1719, i8*, %union.tree_node.1651.1740*, i32, i32, i32, i32, i64 }
%union.edge_def_insns.1630.1719 = type { %struct.gimple_seq_d.1629.1718* }
%struct.loop.1634.1723 = type opaque
%struct.et_node.1635.1724 = type opaque
%union.basic_block_il_dependent.1637.1726 = type { %struct.gimple_bb_info.1636.1725* }
%struct.gimple_bb_info.1636.1725 = type { %struct.gimple_seq_d.1629.1718*, %struct.gimple_seq_d.1629.1718* }
%struct.VEC_basic_block_gc.1640.1729 = type { %struct.VEC_basic_block_base.1639.1728 }
%struct.VEC_basic_block_base.1639.1728 = type { i32, i32, [1 x %struct.basic_block_def.1638.1727*] }
%struct.gimple_seq_d.1629.1718 = type { %struct.gimple_seq_node_d.1628.1717*, %struct.gimple_seq_node_d.1628.1717*, %struct.gimple_seq_d.1629.1718* }
%struct.gimple_seq_node_d.1628.1717 = type { %union.gimple_statement_d.1627.1716*, %struct.gimple_seq_node_d.1628.1717*, %struct.gimple_seq_node_d.1628.1717* }
%union.gimple_statement_d.1627.1716 = type { %struct.gimple_statement_phi.1626.1715 }
%struct.gimple_statement_phi.1626.1715 = type { %struct.gimple_statement_base.1622.1711, i32, i32, %union.tree_node.1651.1740*, [1 x %struct.phi_arg_d.1625.1714] }
%struct.gimple_statement_base.1622.1711 = type { i32, i32, i32, i32, %struct.basic_block_def.1638.1727*, %union.tree_node.1651.1740* }
%struct.phi_arg_d.1625.1714 = type { %struct.ssa_use_operand_d.1624.1713, %union.tree_node.1651.1740*, i32 }
%struct.ssa_use_operand_d.1624.1713 = type { %struct.ssa_use_operand_d.1624.1713*, %struct.ssa_use_operand_d.1624.1713*, %union.anon.1623.1712, %union.tree_node.1651.1740** }
%union.anon.1623.1712 = type { %union.gimple_statement_d.1627.1716* }
%struct.gimple_df.1642.1731 = type opaque
%struct.loops.1643.1732 = type opaque
%struct.machine_function.1647.1736 = type { %struct.stack_local_entry.1645.1734*, i8*, i32, i32, [4 x i32], i32, %struct.machine_cfa_state.1646.1735, i32, i8 }
%struct.stack_local_entry.1645.1734 = type opaque
%struct.machine_cfa_state.1646.1735 = type { %struct.rtx_def.1617.1706*, i64 }
%struct.language_function.1648.1737 = type opaque
%struct.htab.1644.1733 = type { i32 (i8*)*, i32 (i8*, i8*)*, void (i8*)*, i8**, i64, i64, i64, i32, i32, i8* (i64, i64)*, void (i8*)*, i8*, i8* (i8*, i64, i64)*, void (i8*, i8*)*, i32 }

; Function Attrs: nounwind uwtable
define void @native_encode_expr(%union.tree_node.1651.1740* nocapture readonly %expr) local_unnamed_addr {
entry:
  switch i16 undef, label %return [
    i16 23, label %sw.bb
    i16 24, label %sw.bb1
    i16 26, label %sw.bb3
    i16 27, label %sw.bb5
    i16 28, label %sw.bb7
  ]

sw.bb:                                            ; preds = %entry
  br i1 undef, label %cond.true.i36, label %cond.false.i42

cond.true.i36:                                    ; preds = %sw.bb
  br label %cond.end.i48

cond.false.i42:                                   ; preds = %sw.bb
  br label %cond.end.i48

cond.end.i48:                                     ; preds = %cond.false.i42, %cond.true.i36
  br i1 undef, label %return, label %for.cond.preheader.i

for.cond.preheader.i:                             ; preds = %cond.end.i48
  br i1 undef, label %return, label %for.body.lr.ph.i

for.body.lr.ph.i:                                 ; preds = %for.cond.preheader.i
  %int_cst13.i = getelementptr inbounds %union.tree_node.1651.1740, %union.tree_node.1651.1740* %expr, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 1
  %high.i = getelementptr inbounds i32, i32* %int_cst13.i, i64 2
  %0 = bitcast i32* %high.i to i64*
  br label %for.body.i49

for.body.i49:                                     ; preds = %if.end27.i, %for.body.lr.ph.i
  %indvars.iv.i = phi i64 [ 0, %for.body.lr.ph.i ], [ %indvars.iv.next.i, %if.end27.i ]
  br i1 undef, label %if.then12.i, label %if.else.i52

if.then12.i:                                      ; preds = %for.body.i49
  br label %if.end20.i

if.else.i52:                                      ; preds = %for.body.i49
  %1 = load i64, i64* %0, align 8
  br label %if.end20.i

if.end20.i:                                       ; preds = %if.else.i52, %if.then12.i
  br i1 undef, label %if.then23.i, label %if.end27.i

if.then23.i:                                      ; preds = %if.end20.i
  br label %if.end27.i

if.end27.i:                                       ; preds = %if.then23.i, %if.end20.i
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next.i to i32
  %exitcond = icmp eq i32 %lftr.wideiv, 50
  br i1 %exitcond, label %return.loopexit, label %for.body.i49

sw.bb1:                                           ; preds = %entry
  br i1 undef, label %cond.true.i, label %cond.false.i27

cond.true.i:                                      ; preds = %sw.bb1
  br label %cond.end.i

cond.false.i27:                                   ; preds = %sw.bb1
  br label %cond.end.i

cond.end.i:                                       ; preds = %cond.false.i27, %cond.true.i
  br i1 undef, label %native_encode_real.exit, label %if.end.i30

if.end.i30:                                       ; preds = %cond.end.i
  br i1 undef, label %cond.true14.i, label %cond.false16.i

cond.true14.i:                                    ; preds = %if.end.i30
  br label %cond.end22.i

cond.false16.i:                                   ; preds = %if.end.i30
  br label %cond.end22.i

cond.end22.i:                                     ; preds = %cond.false16.i, %cond.true14.i
  br i1 undef, label %native_encode_real.exit, label %for.body.i.preheader

for.body.i.preheader:                             ; preds = %cond.end22.i
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.i.preheader
  br i1 undef, label %for.body.i, label %native_encode_real.exit.loopexit

native_encode_real.exit.loopexit:                 ; preds = %for.body.i
  br label %native_encode_real.exit

native_encode_real.exit:                          ; preds = %native_encode_real.exit.loopexit, %cond.end22.i, %cond.end.i
  br label %return

sw.bb3:                                           ; preds = %entry
  br i1 undef, label %return, label %if.end.i62

if.end.i62:                                       ; preds = %sw.bb3
  br label %return

sw.bb5:                                           ; preds = %entry
  br i1 undef, label %cond.true.i68, label %cond.false.i71

cond.true.i68:                                    ; preds = %sw.bb5
  br label %cond.end.i75

cond.false.i71:                                   ; preds = %sw.bb5
  br label %cond.end.i75

cond.end.i75:                                     ; preds = %cond.false.i71, %cond.true.i68
  br i1 undef, label %for.body.i78.lr.ph, label %return

for.body.i78.lr.ph:                               ; preds = %cond.end.i75
  br label %for.body.i78

for.body.i78:                                     ; preds = %if.end33.i, %for.body.i78.lr.ph
  br i1 undef, label %if.else25.i, label %if.then.i

if.then.i:                                        ; preds = %for.body.i78
  br i1 undef, label %if.else25.i, label %if.then19.i

if.then19.i:                                      ; preds = %if.then.i
  br i1 undef, label %if.end33.i, label %return.loopexit96

if.else25.i:                                      ; preds = %if.then.i, %for.body.i78
  br i1 undef, label %return.loopexit96, label %if.end29.i

if.end29.i:                                       ; preds = %if.else25.i
  br label %if.end33.i

if.end33.i:                                       ; preds = %if.end29.i, %if.then19.i
  br i1 undef, label %for.body.i78, label %return.loopexit96

sw.bb7:                                           ; preds = %entry
  br i1 undef, label %lor.lhs.false.i, label %return

lor.lhs.false.i:                                  ; preds = %sw.bb7
  br i1 undef, label %cond.false.i, label %return

cond.false.i:                                     ; preds = %lor.lhs.false.i
  br i1 undef, label %lor.lhs.false29.i, label %return

lor.lhs.false29.i:                                ; preds = %cond.false.i
  br i1 undef, label %return, label %if.end.i

if.end.i:                                         ; preds = %lor.lhs.false29.i
  br i1 undef, label %return, label %if.end39.i

if.end39.i:                                       ; preds = %if.end.i
  br i1 undef, label %if.then43.i, label %if.else.i

if.then43.i:                                      ; preds = %if.end39.i
  br label %if.end56.i

if.else.i:                                        ; preds = %if.end39.i
  br label %if.end56.i

if.end56.i:                                       ; preds = %if.else.i, %if.then43.i
  br label %return

return.loopexit:                                  ; preds = %if.end27.i
  br label %return

return.loopexit96:                                ; preds = %if.end33.i, %if.else25.i, %if.then19.i
  br label %return

return:                                           ; preds = %return.loopexit96, %return.loopexit, %if.end56.i, %if.end.i, %lor.lhs.false29.i, %cond.false.i, %lor.lhs.false.i, %sw.bb7, %cond.end.i75, %if.end.i62, %sw.bb3, %native_encode_real.exit, %for.cond.preheader.i, %cond.end.i48, %entry
  ret void
}

