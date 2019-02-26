; RUN: opt < %s -slp-vectorizer -verify -S
; ModuleID = 'qfbcursor.cpp'
source_filename = "qfbcursor.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

module asm ".section .qtversion, \22aG\22, @progbits, qt_version_tag, comdat"
module asm ".align 8"
module asm ".quad qt_version_tag@GOT"
module asm ".long ((5<<16)|(11<<8)|(2))"
module asm ".align 8"
module asm ".previous"

%struct.QMetaObject = type { %struct.anon }
%struct.anon = type { %struct.QMetaObject*, %struct.QArrayData*, i32*, void (%class.QObject*, i32, i32, i8**)*, %struct.QMetaObject**, i8* }
%struct.QArrayData = type { %"class.QtPrivate::RefCount", i32, i32, i64 }
%"class.QtPrivate::RefCount" = type { %class.QBasicAtomicInteger }
%class.QBasicAtomicInteger = type { %"struct.std::atomic" }
%"struct.std::atomic" = type { %"struct.std::__atomic_base" }
%"struct.std::__atomic_base" = type { i32 }
%class.QObject = type { i32 (...)**, %class.QScopedPointer }
%class.QScopedPointer = type { %class.QObjectData* }
%class.QObjectData = type { i32 (...)**, %class.QObject*, %class.QObject*, %class.QList, i32, i32, %struct.QDynamicMetaObjectData* }
%class.QList = type { %union.anon }
%union.anon = type { %struct.QListData }
%struct.QListData = type { %"struct.QListData::Data"* }
%"struct.QListData::Data" = type { %"class.QtPrivate::RefCount", i32, i32, i32, [1 x i8*] }
%struct.QDynamicMetaObjectData = type { i32 (...)** }
%class.QFbCursor = type { %class.QPlatformCursor, i8, %class.QFbScreen*, %class.QRect, %class.QRect, i8, i8, %class.QPlatformCursorImage*, %class.QFbCursorDeviceListener*, %class.QPoint }
%class.QPlatformCursor = type { %class.QObject }
%class.QRect = type { i32, i32, i32, i32 }
%class.QPlatformCursorImage = type { %class.QImage, %class.QPoint }
%class.QImage = type { %class.QPaintDevice, %struct.QImageData* }
%class.QPaintDevice = type { i32 (...)**, i16, %class.QPaintDevicePrivate* }
%class.QPaintDevicePrivate = type opaque
%struct.QImageData = type opaque
%class.QFbCursorDeviceListener = type { %class.QObject, %class.QFbCursor* }
%class.QPoint = type { i32, i32 }
%class.QFbScreen = type { %class.QObject, %class.QPlatformScreen, %class.QList.1, %class.QRegion, i8, %class.QFbCursor*, %class.QRect, i32, i32, %class.QSizeF, %class.QImage, %class.QPainter*, %class.QList.6 }
%class.QPlatformScreen = type { i32 (...)**, %class.QScopedPointer.0 }
%class.QScopedPointer.0 = type { %class.QPlatformScreenPrivate* }
%class.QPlatformScreenPrivate = type opaque
%class.QList.1 = type { %union.anon.4 }
%union.anon.4 = type { %struct.QListData }
%class.QRegion = type { %"struct.QRegion::QRegionData"* }
%"struct.QRegion::QRegionData" = type { %"class.QtPrivate::RefCount", %struct.QRegionPrivate* }
%struct.QRegionPrivate = type opaque
%class.QSizeF = type { double, double }
%class.QPainter = type { %class.QScopedPointer.5 }
%class.QScopedPointer.5 = type { %class.QPainterPrivate* }
%class.QPainterPrivate = type opaque
%class.QList.6 = type { %union.anon.9 }
%union.anon.9 = type { %struct.QListData }
%class.QInputDeviceManager = type { %class.QObject }
%class.QByteArray = type { %struct.QTypedArrayData* }
%struct.QTypedArrayData = type { %struct.QArrayData }
%"class.QMetaObject::Connection" = type { i8* }
%"class.QtPrivate::QSlotObjectBase" = type { %class.QAtomicInt, void (i32, %"class.QtPrivate::QSlotObjectBase"*, %class.QObject*, i8**, i8*)* }
%class.QAtomicInt = type { %class.QAtomicInteger }
%class.QAtomicInteger = type { %class.QBasicAtomicInteger }
%class.QMouseEvent = type <{ %class.QInputEvent, %class.QPointF, %class.QPointF, %class.QPointF, i32, %class.QFlags.10, i32, %class.QVector2D, [4 x i8] }>
%class.QInputEvent = type { %class.QEvent.base, %class.QFlags, i64 }
%class.QEvent.base = type <{ i32 (...)**, %class.QEventPrivate*, i16, i16 }>
%class.QEventPrivate = type opaque
%class.QFlags = type { i32 }
%class.QPointF = type { double, double }
%class.QFlags.10 = type { i32 }
%class.QVector2D = type { float, float }
%class.QRectF = type { double, double, double, double }
%class.QCursor = type { %class.QCursorData* }
%class.QCursorData = type opaque
%class.QWindow = type { %class.QObject, %class.QSurface }
%class.QSurface = type { i32 (...)**, i32, %class.QSurfacePrivate* }
%class.QSurfacePrivate = type opaque
%class.QPixmap = type { %class.QPaintDevice, %class.QExplicitlySharedDataPointer }
%class.QExplicitlySharedDataPointer = type { %class.QPlatformPixmap* }
%class.QPlatformPixmap = type { i32 (...)**, i32, i32, i32, i8, %class.QAtomicInt, i32, i32, i32, i32, i32 }

$_ZN9QtPrivate11QSlotObjectIM23QFbCursorDeviceListenerFvN19QInputDeviceManager10DeviceTypeEENS_4ListIIS3_EEEvE4implEiPNS_15QSlotObjectBaseEP7QObjectPPvPb = comdat any

@_ZTV9QFbCursor = external dso_local unnamed_addr constant { [25 x i8*] }, align 8
@.str = private unnamed_addr constant [21 x i8] c"QT_QPA_FB_HIDECURSOR\00", align 1
@_ZTV23QFbCursorDeviceListener = external dso_local unnamed_addr constant { [14 x i8*] }, align 8
@_ZN19QInputDeviceManager16staticMetaObjectE = external dso_local global %struct.QMetaObject, align 8

@_ZN9QFbCursorC1EP9QFbScreen = dso_local unnamed_addr alias void (%class.QFbCursor*, %class.QFbScreen*), void (%class.QFbCursor*, %class.QFbScreen*)* @_ZN9QFbCursorC2EP9QFbScreen
@_ZN9QFbCursorD1Ev = dso_local unnamed_addr alias void (%class.QFbCursor*), void (%class.QFbCursor*)* @_ZN9QFbCursorD2Ev

; Function Attrs: uwtable
define dso_local zeroext i1 @_ZNK23QFbCursorDeviceListener8hasMouseEv(%class.QFbCursorDeviceListener* nocapture readnone %this) local_unnamed_addr #0 align 2 {
entry:
  %call = tail call %class.QInputDeviceManager* @_ZN22QGuiApplicationPrivate18inputDeviceManagerEv()
  %call2 = tail call i32 @_ZNK19QInputDeviceManager11deviceCountENS_10DeviceTypeE(%class.QInputDeviceManager* %call, i32 1)
  %cmp = icmp sgt i32 %call2, 0
  ret i1 %cmp
}

declare dso_local %class.QInputDeviceManager* @_ZN22QGuiApplicationPrivate18inputDeviceManagerEv() local_unnamed_addr #1

declare dso_local i32 @_ZNK19QInputDeviceManager11deviceCountENS_10DeviceTypeE(%class.QInputDeviceManager*, i32) local_unnamed_addr #1

; Function Attrs: uwtable
define dso_local void @_ZN23QFbCursorDeviceListener19onDeviceListChangedEN19QInputDeviceManager10DeviceTypeE(%class.QFbCursorDeviceListener* nocapture readonly %this, i32 %type) #0 align 2 {
entry:
  %cmp = icmp eq i32 %type, 1
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %m_cursor = getelementptr inbounds %class.QFbCursorDeviceListener, %class.QFbCursorDeviceListener* %this, i64 0, i32 1, !intel-tbaa !2
  %0 = load %class.QFbCursor*, %class.QFbCursor** %m_cursor, align 8, !tbaa !2
  tail call void @_ZN9QFbCursor17updateMouseStatusEv(%class.QFbCursor* %0)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; Function Attrs: uwtable
define dso_local void @_ZN9QFbCursor17updateMouseStatusEv(%class.QFbCursor* %this) local_unnamed_addr #0 align 2 personality i32 (...)* @__gxx_personality_v0 {
entry:
  %ref.tmp = alloca { i64, i64 }, align 8
  %tmpcast = bitcast { i64, i64 }* %ref.tmp to %class.QRect*
  %call.i = tail call %class.QInputDeviceManager* @_ZN22QGuiApplicationPrivate18inputDeviceManagerEv()
  %call2.i = tail call i32 @_ZNK19QInputDeviceManager11deviceCountENS_10DeviceTypeE(%class.QInputDeviceManager* %call.i, i32 1)
  %cmp.i = icmp sgt i32 %call2.i, 0
  %mVisible = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 1, !intel-tbaa !7
  %frombool = zext i1 %cmp.i to i8
  store i8 %frombool, i8* %mVisible, align 8, !tbaa !7
  %mScreen = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 2, !intel-tbaa !16
  %0 = load %class.QFbScreen*, %class.QFbScreen** %mScreen, align 8, !tbaa !16
  %1 = bitcast { i64, i64 }* %ref.tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %1) #8
  br i1 %cmp.i, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %mCursorImage.i = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 7, !intel-tbaa !17
  %2 = load %class.QPlatformCursorImage*, %class.QPlatformCursorImage** %mCursorImage.i, align 8, !tbaa !17
  %cursorImage.i.i = getelementptr inbounds %class.QPlatformCursorImage, %class.QPlatformCursorImage* %2, i64 0, i32 0, !intel-tbaa !18
  %call2.i7 = tail call { i64, i64 } @_ZNK6QImage4rectEv(%class.QImage* %cursorImage.i.i)
  %3 = extractvalue { i64, i64 } %call2.i7, 0
  %ref.tmp.sroa.0.sroa.4.0.extract.shift.i = lshr i64 %3, 32
  %ref.tmp.sroa.0.sroa.4.0.extract.trunc.i = trunc i64 %ref.tmp.sroa.0.sroa.4.0.extract.shift.i to i32
  %4 = extractvalue { i64, i64 } %call2.i7, 1
  %5 = load %class.QPlatformCursorImage*, %class.QPlatformCursorImage** %mCursorImage.i, align 8, !tbaa !17
  %retval.sroa.0.0..sroa_idx.i.i = getelementptr inbounds %class.QPlatformCursorImage, %class.QPlatformCursorImage* %5, i64 0, i32 1
  %retval.sroa.0.0..sroa_cast.i.i = bitcast %class.QPoint* %retval.sroa.0.0..sroa_idx.i.i to i64*
  %retval.sroa.0.0.copyload.i.i = load i64, i64* %retval.sroa.0.0..sroa_cast.i.i, align 8
  %ref.tmp3.sroa.4.0.extract.shift.i = lshr i64 %retval.sroa.0.0.copyload.i.i, 32
  %ref.tmp7.sroa.3.0.extract.trunc.i = trunc i64 %ref.tmp3.sroa.4.0.extract.shift.i to i32
  %add.i41.i = sub i64 %3, %retval.sroa.0.0.copyload.i.i
  %add2.i.i = sub i32 %ref.tmp.sroa.0.sroa.4.0.extract.trunc.i, %ref.tmp7.sroa.3.0.extract.trunc.i
  %add4.i44.i = sub i64 %4, %retval.sroa.0.0.copyload.i.i
  %ref.tmp.sroa.5.12.extract.shift.i = lshr i64 %4, 32
  %ref.tmp.sroa.5.12.extract.trunc.i = trunc i64 %ref.tmp.sroa.5.12.extract.shift.i to i32
  %add5.i.i = sub i32 %ref.tmp.sroa.5.12.extract.trunc.i, %ref.tmp7.sroa.3.0.extract.trunc.i
  %retval.sroa.0.0.extract.trunc.i = trunc i64 %add.i41.i to i32
  %retval.sroa.10.8.extract.trunc.i = trunc i64 %add4.i44.i to i32
  %xp.i.i30.i = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 9, i32 0, !intel-tbaa !22
  %6 = load i32, i32* %xp.i.i30.i, align 4, !tbaa !22
  %add.i32.i = add nsw i32 %6, %retval.sroa.0.0.extract.trunc.i
  %yp.i15.i33.i = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 9, i32 1, !intel-tbaa !23
  %7 = load i32, i32* %yp.i15.i33.i, align 4, !tbaa !23
  %add4.i35.i = add nsw i32 %add2.i.i, %7
  %add7.i37.i = add nsw i32 %6, %retval.sroa.10.8.extract.trunc.i
  %add10.i39.i = add nsw i32 %add5.i.i, %7
  %8 = load %class.QFbScreen*, %class.QFbScreen** %mScreen, align 8, !tbaa !16
  %9 = bitcast %class.QFbScreen* %8 to { i64, i64 } (%class.QFbScreen*)***
  %vtable.i = load { i64, i64 } (%class.QFbScreen*)**, { i64, i64 } (%class.QFbScreen*)*** %9, align 8, !tbaa !24
  %vfn.i = getelementptr inbounds { i64, i64 } (%class.QFbScreen*)*, { i64, i64 } (%class.QFbScreen*)** %vtable.i, i64 13
  %10 = load { i64, i64 } (%class.QFbScreen*)*, { i64, i64 } (%class.QFbScreen*)** %vfn.i, align 8
  %call14.i = tail call { i64, i64 } %10(%class.QFbScreen* %8)
  %11 = extractvalue { i64, i64 } %call14.i, 0
  %ref.tmp13.sroa.0.sroa.4.0.extract.shift.i = lshr i64 %11, 32
  %ref.tmp13.sroa.0.sroa.4.0.extract.trunc.i = trunc i64 %ref.tmp13.sroa.0.sroa.4.0.extract.shift.i to i32
  %12 = trunc i64 %11 to i32
  %add.i.i = sub i32 %add.i32.i, %12
  %add4.i.i = sub i32 %add4.i35.i, %ref.tmp13.sroa.0.sroa.4.0.extract.trunc.i
  %add7.i.i = sub i32 %add7.i37.i, %12
  %add10.i.i = sub i32 %add10.i39.i, %ref.tmp13.sroa.0.sroa.4.0.extract.trunc.i
  %retval.sroa.6.0.insert.ext.i = zext i32 %add4.i.i to i64
  %retval.sroa.6.0.insert.shift.i = shl nuw i64 %retval.sroa.6.0.insert.ext.i, 32
  %retval.sroa.0.0.insert.ext.i = zext i32 %add.i.i to i64
  %retval.sroa.0.0.insert.insert.i = or i64 %retval.sroa.6.0.insert.shift.i, %retval.sroa.0.0.insert.ext.i
  %retval.sroa.16.8.insert.ext.i = zext i32 %add10.i.i to i64
  %retval.sroa.16.8.insert.shift.i = shl nuw i64 %retval.sroa.16.8.insert.ext.i, 32
  %retval.sroa.10.8.insert.ext.i = zext i32 %add7.i.i to i64
  %retval.sroa.10.8.insert.insert.i = or i64 %retval.sroa.16.8.insert.shift.i, %retval.sroa.10.8.insert.ext.i
  br label %cond.end

cond.false:                                       ; preds = %entry
  %13 = bitcast %class.QFbCursor* %this to { i64, i64 } (%class.QFbCursor*)***
  %vtable = load { i64, i64 } (%class.QFbCursor*)**, { i64, i64 } (%class.QFbCursor*)*** %13, align 8, !tbaa !24
  %vfn = getelementptr inbounds { i64, i64 } (%class.QFbCursor*)*, { i64, i64 } (%class.QFbCursor*)** %vtable, i64 22
  %14 = load { i64, i64 } (%class.QFbCursor*)*, { i64, i64 } (%class.QFbCursor*)** %vfn, align 8
  %call4 = tail call { i64, i64 } %14(%class.QFbCursor* nonnull %this)
  %15 = extractvalue { i64, i64 } %call4, 0
  %16 = extractvalue { i64, i64 } %call4, 1
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %retval.sroa.0.0.insert.insert.i.sink = phi i64 [ %15, %cond.false ], [ %retval.sroa.0.0.insert.insert.i, %cond.true ]
  %retval.sroa.10.8.insert.insert.i.sink = phi i64 [ %16, %cond.false ], [ %retval.sroa.10.8.insert.insert.i, %cond.true ]
  %17 = getelementptr inbounds { i64, i64 }, { i64, i64 }* %ref.tmp, i64 0, i32 0
  store i64 %retval.sroa.0.0.insert.insert.i.sink, i64* %17, align 8
  %18 = getelementptr inbounds { i64, i64 }, { i64, i64 }* %ref.tmp, i64 0, i32 1
  store i64 %retval.sroa.10.8.insert.insert.i.sink, i64* %18, align 8
  %19 = bitcast %class.QFbScreen* %0 to void (%class.QFbScreen*, %class.QRect*)***
  %vtable5 = load void (%class.QFbScreen*, %class.QRect*)**, void (%class.QFbScreen*, %class.QRect*)*** %19, align 8, !tbaa !24
  %vfn6 = getelementptr inbounds void (%class.QFbScreen*, %class.QRect*)*, void (%class.QFbScreen*, %class.QRect*)** %vtable5, i64 26
  %20 = load void (%class.QFbScreen*, %class.QRect*)*, void (%class.QFbScreen*, %class.QRect*)** %vfn6, align 8
  call void %20(%class.QFbScreen* %0, %class.QRect* nonnull dereferenceable(16) %tmpcast)
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %1) #8
  ret void
}

; Function Attrs: uwtable
define dso_local void @_ZN9QFbCursorC2EP9QFbScreen(%class.QFbCursor* %this, %class.QFbScreen* %screen) unnamed_addr #0 align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %signal.addr.i = alloca <2 x i64>, align 16
  %slot.addr.i = alloca <2 x i64>, align 16
  %hideCursorVal = alloca %class.QByteArray, align 8
  %agg.tmp.ensured = alloca %"class.QMetaObject::Connection", align 8
  %0 = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 0
  tail call void @_ZN15QPlatformCursorC2Ev(%class.QPlatformCursor* %0)
  %1 = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [25 x i8*] }, { [25 x i8*] }* @_ZTV9QFbCursor, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %1, align 8, !tbaa !24
  %mVisible = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 1, !intel-tbaa !7
  store i8 1, i8* %mVisible, align 8, !tbaa !7
  %mScreen = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 2, !intel-tbaa !16
  store %class.QFbScreen* %screen, %class.QFbScreen** %mScreen, align 8, !tbaa !16
  %x1.i = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 3, i32 0, !intel-tbaa !26
  %2 = bitcast i32* %x1.i to <4 x i32>*
  store <4 x i32> <i32 0, i32 0, i32 -1, i32 -1>, <4 x i32>* %2, align 4, !tbaa !27
  %x1.i34 = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 4, i32 0, !intel-tbaa !28
  %3 = bitcast i32* %x1.i34 to <4 x i32>*
  store <4 x i32> <i32 0, i32 0, i32 -1, i32 -1>, <4 x i32>* %3, align 4, !tbaa !27
  %mDirty = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 5, !intel-tbaa !29
  store i8 0, i8* %mDirty, align 8, !tbaa !29
  %mOnScreen = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 6, !intel-tbaa !30
  store i8 0, i8* %mOnScreen, align 1, !tbaa !30
  %mCursorImage = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 7, !intel-tbaa !17
  %mDeviceListener = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 8, !intel-tbaa !31
  %4 = bitcast %class.QByteArray* %hideCursorVal to i8*
  %5 = bitcast %class.QPlatformCursorImage** %mCursorImage to i8*
  call void @llvm.memset.p0i8.i64(i8* nonnull align 8 %5, i8 0, i64 24, i1 false)
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %4) #8
  invoke void @_Z7qgetenvPKc(%class.QByteArray* nonnull sret %hideCursorVal, i8* getelementptr inbounds ([21 x i8], [21 x i8]* @.str, i64 0, i64 0))
          to label %invoke.cont3 unwind label %lpad2

invoke.cont3:                                     ; preds = %entry
  %6 = bitcast %class.QByteArray* %hideCursorVal to %struct.QArrayData**
  %7 = load %struct.QArrayData*, %struct.QArrayData** %6, align 8, !tbaa !32
  %size.i = getelementptr inbounds %struct.QArrayData, %struct.QArrayData* %7, i64 0, i32 1, !intel-tbaa !35
  %8 = load i32, i32* %size.i, align 4, !tbaa !35
  %cmp.i = icmp eq i32 %8, 0
  br i1 %cmp.i, label %if.endthread-pre-split, label %if.then

if.then:                                          ; preds = %invoke.cont3
  %call7 = invoke i32 @_ZNK10QByteArray5toIntEPbi(%class.QByteArray* nonnull %hideCursorVal, i8* null, i32 10)
          to label %invoke.cont6 unwind label %lpad4

invoke.cont6:                                     ; preds = %if.then
  %cmp = icmp eq i32 %call7, 0
  %frombool = zext i1 %cmp to i8
  store i8 %frombool, i8* %mVisible, align 8, !tbaa !7
  br label %if.end

lpad2:                                            ; preds = %entry
  %9 = landingpad { i8*, i32 }
          cleanup
  %10 = extractvalue { i8*, i32 } %9, 0
  %11 = extractvalue { i8*, i32 } %9, 1
  br label %ehcleanup29

lpad4:                                            ; preds = %invoke.cont15, %call3.i.noexc, %invoke.cont23, %invoke.cont26, %invoke.cont21, %invoke.cont17, %if.end11, %if.then
  %12 = landingpad { i8*, i32 }
          cleanup
  %13 = extractvalue { i8*, i32 } %12, 0
  %14 = extractvalue { i8*, i32 } %12, 1
  br label %ehcleanup

if.endthread-pre-split:                           ; preds = %invoke.cont3
  %.pr = load i8, i8* %mVisible, align 8, !tbaa !7
  br label %if.end

if.end:                                           ; preds = %if.endthread-pre-split, %invoke.cont6
  %15 = phi i8 [ %.pr, %if.endthread-pre-split ], [ %frombool, %invoke.cont6 ]
  %tobool = icmp eq i8 %15, 0
  br i1 %tobool, label %cleanup, label %if.end11

if.end11:                                         ; preds = %if.end
  %call13 = invoke i8* @_Znwm(i64 40) #9
          to label %invoke.cont12 unwind label %lpad4

invoke.cont12:                                    ; preds = %if.end11
  %16 = bitcast i8* %call13 to %class.QPlatformCursorImage*
  %cursorImage.i = bitcast i8* %call13 to %class.QImage*
  call void @_ZN6QImageC1Ev(%class.QImage* nonnull %cursorImage.i) #8
  %xp.i.i = getelementptr inbounds i8, i8* %call13, i64 32
  %17 = bitcast i8* %xp.i.i to i32*
  store i32 0, i32* %17, align 4, !tbaa !41
  %yp.i.i = getelementptr inbounds i8, i8* %call13, i64 36
  %18 = bitcast i8* %yp.i.i to i32*
  store i32 0, i32* %18, align 4, !tbaa !42
  invoke void @_ZN20QPlatformCursorImage3setEPKhS1_iiii(%class.QPlatformCursorImage* nonnull %16, i8* null, i8* null, i32 0, i32 0, i32 0, i32 0)
          to label %invoke.cont15 unwind label %lpad.i

lpad.i:                                           ; preds = %invoke.cont12
  %19 = landingpad { i8*, i32 }
          cleanup
  call void @_ZN6QImageD1Ev(%class.QImage* nonnull %cursorImage.i) #8
  %20 = extractvalue { i8*, i32 } %19, 0
  %21 = extractvalue { i8*, i32 } %19, 1
  call void @_ZdlPv(i8* nonnull %call13) #10
  br label %ehcleanup

invoke.cont15:                                    ; preds = %invoke.cont12
  %22 = bitcast %class.QPlatformCursorImage** %mCursorImage to i8**
  store i8* %call13, i8** %22, align 8, !tbaa !17
  invoke void @_ZN20QPlatformCursorImage3setEN2Qt11CursorShapeE(%class.QPlatformCursorImage* nonnull %16, i32 0)
          to label %invoke.cont17 unwind label %lpad4

invoke.cont17:                                    ; preds = %invoke.cont15
  %call19 = invoke i8* @_Znwm(i64 24) #9
          to label %invoke.cont18 unwind label %lpad4

invoke.cont18:                                    ; preds = %invoke.cont17
  %23 = bitcast i8* %call19 to %class.QObject*
  invoke void @_ZN7QObjectC2EPS_(%class.QObject* nonnull %23, %class.QObject* null)
          to label %invoke.cont21 unwind label %lpad20

invoke.cont21:                                    ; preds = %invoke.cont18
  %24 = bitcast i8* %call19 to i32 (...)***
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [14 x i8*] }, { [14 x i8*] }* @_ZTV23QFbCursorDeviceListener, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %24, align 8, !tbaa !24
  %m_cursor.i = getelementptr inbounds i8, i8* %call19, i64 16
  %25 = bitcast i8* %m_cursor.i to %class.QFbCursor**
  store %class.QFbCursor* %this, %class.QFbCursor** %25, align 8, !tbaa !2
  %26 = bitcast %class.QFbCursorDeviceListener** %mDeviceListener to i8**
  store i8* %call19, i8** %26, align 8, !tbaa !31
  %call24 = invoke %class.QInputDeviceManager* @_ZN22QGuiApplicationPrivate18inputDeviceManagerEv()
          to label %invoke.cont23 unwind label %lpad4

invoke.cont23:                                    ; preds = %invoke.cont21
  %27 = load %class.QFbCursorDeviceListener*, %class.QFbCursorDeviceListener** %mDeviceListener, align 8, !tbaa !31
  %28 = bitcast <2 x i64>* %signal.addr.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %28)
  %29 = bitcast <2 x i64>* %slot.addr.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %29)
  store <2 x i64> <i64 ptrtoint (void (%class.QInputDeviceManager*, i32)* @_ZN19QInputDeviceManager17deviceListChangedENS_10DeviceTypeE to i64), i64 0>, <2 x i64>* %signal.addr.i, align 16, !tbaa !43, !noalias !44
  store <2 x i64> <i64 ptrtoint (void (%class.QFbCursorDeviceListener*, i32)* @_ZN23QFbCursorDeviceListener19onDeviceListChangedEN19QInputDeviceManager10DeviceTypeE to i64), i64 0>, <2 x i64>* %slot.addr.i, align 16, !tbaa !43, !noalias !44
  %call3.i46 = invoke i8* @_Znwm(i64 32) #9
          to label %call3.i.noexc unwind label %lpad4

call3.i.noexc:                                    ; preds = %invoke.cont23
  %_M_i.i.i.i.i.i.i.i.i = bitcast i8* %call3.i46 to i32*
  store i32 1, i32* %_M_i.i.i.i.i.i.i.i.i, align 4, !tbaa !47, !noalias !44
  %m_impl.i.i.i = getelementptr inbounds i8, i8* %call3.i46, i64 8
  %30 = bitcast i8* %m_impl.i.i.i to void (i32, %"class.QtPrivate::QSlotObjectBase"*, %class.QObject*, i8**, i8*)**
  store void (i32, %"class.QtPrivate::QSlotObjectBase"*, %class.QObject*, i8**, i8*)* @_ZN9QtPrivate11QSlotObjectIM23QFbCursorDeviceListenerFvN19QInputDeviceManager10DeviceTypeEENS_4ListIIS3_EEEvE4implEiPNS_15QSlotObjectBaseEP7QObjectPPvPb, void (i32, %"class.QtPrivate::QSlotObjectBase"*, %class.QObject*, i8**, i8*)** %30, align 8, !tbaa !49, !noalias !44
  %function.repack.i.i = getelementptr inbounds i8, i8* %call3.i46, i64 16
  %31 = bitcast i8* %function.repack.i.i to <2 x i64>*
  store <2 x i64> <i64 ptrtoint (void (%class.QFbCursorDeviceListener*, i32)* @_ZN23QFbCursorDeviceListener19onDeviceListChangedEN19QInputDeviceManager10DeviceTypeE to i64), i64 0>, <2 x i64>* %31, align 8, !tbaa !53, !noalias !44
  %32 = bitcast <2 x i64>* %slot.addr.i to i8**
  %33 = getelementptr inbounds %class.QFbCursorDeviceListener, %class.QFbCursorDeviceListener* %27, i64 0, i32 0
  %34 = bitcast <2 x i64>* %signal.addr.i to i8**
  %35 = getelementptr inbounds %class.QInputDeviceManager, %class.QInputDeviceManager* %call24, i64 0, i32 0
  %36 = bitcast i8* %call3.i46 to %"class.QtPrivate::QSlotObjectBase"*
  invoke void @_ZN7QObject11connectImplEPKS_PPvS1_S3_PN9QtPrivate15QSlotObjectBaseEN2Qt14ConnectionTypeEPKiPK11QMetaObject(%"class.QMetaObject::Connection"* nonnull sret %agg.tmp.ensured, %class.QObject* %35, i8** nonnull %34, %class.QObject* %33, i8** nonnull %32, %"class.QtPrivate::QSlotObjectBase"* nonnull %36, i32 0, i32* null, %struct.QMetaObject* nonnull @_ZN19QInputDeviceManager16staticMetaObjectE)
          to label %invoke.cont26 unwind label %lpad4

invoke.cont26:                                    ; preds = %call3.i.noexc
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %28)
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %29)
  call void @_ZN11QMetaObject10ConnectionD1Ev(%"class.QMetaObject::Connection"* nonnull %agg.tmp.ensured) #8
  invoke void @_ZN9QFbCursor17updateMouseStatusEv(%class.QFbCursor* nonnull %this)
          to label %cleanup unwind label %lpad4

cleanup:                                          ; preds = %invoke.cont26, %if.end
  %37 = load %struct.QArrayData*, %struct.QArrayData** %6, align 8, !tbaa !32
  %_M_i.i.i.i.i.i38 = getelementptr inbounds %struct.QArrayData, %struct.QArrayData* %37, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %38 = load atomic i32, i32* %_M_i.i.i.i.i.i38 monotonic, align 4
  switch i32 %38, label %_ZN9QtPrivate8RefCount5derefEv.exit.i40 [
    i32 0, label %if.then.i43
    i32 -1, label %_ZN10QByteArrayD2Ev.exit44
  ]

_ZN9QtPrivate8RefCount5derefEv.exit.i40:          ; preds = %cleanup
  %39 = atomicrmw sub i32* %_M_i.i.i.i.i.i38, i32 1 seq_cst
  %cmp.i.i.i.i39 = icmp eq i32 %39, 1
  br i1 %cmp.i.i.i.i39, label %_ZN9QtPrivate8RefCount5derefEv.exit.if.then_crit_edge.i42, label %_ZN10QByteArrayD2Ev.exit44

_ZN9QtPrivate8RefCount5derefEv.exit.if.then_crit_edge.i42: ; preds = %_ZN9QtPrivate8RefCount5derefEv.exit.i40
  %.pre.i41 = load %struct.QArrayData*, %struct.QArrayData** %6, align 8, !tbaa !32
  br label %if.then.i43

if.then.i43:                                      ; preds = %_ZN9QtPrivate8RefCount5derefEv.exit.if.then_crit_edge.i42, %cleanup
  %40 = phi %struct.QArrayData* [ %.pre.i41, %_ZN9QtPrivate8RefCount5derefEv.exit.if.then_crit_edge.i42 ], [ %37, %cleanup ]
  call void @_ZN10QArrayData10deallocateEPS_mm(%struct.QArrayData* %40, i64 1, i64 8) #8
  br label %_ZN10QByteArrayD2Ev.exit44

_ZN10QByteArrayD2Ev.exit44:                       ; preds = %cleanup, %_ZN9QtPrivate8RefCount5derefEv.exit.i40, %if.then.i43
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %4) #8
  ret void

lpad20:                                           ; preds = %invoke.cont18
  %41 = landingpad { i8*, i32 }
          cleanup
  %42 = extractvalue { i8*, i32 } %41, 0
  %43 = extractvalue { i8*, i32 } %41, 1
  call void @_ZdlPv(i8* nonnull %call19) #10
  br label %ehcleanup

ehcleanup:                                        ; preds = %lpad20, %lpad.i, %lpad4
  %ehselector.slot.0 = phi i32 [ %14, %lpad4 ], [ %43, %lpad20 ], [ %21, %lpad.i ]
  %exn.slot.0 = phi i8* [ %13, %lpad4 ], [ %42, %lpad20 ], [ %20, %lpad.i ]
  %44 = load %struct.QArrayData*, %struct.QArrayData** %6, align 8, !tbaa !32
  %_M_i.i.i.i.i.i = getelementptr inbounds %struct.QArrayData, %struct.QArrayData* %44, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %45 = load atomic i32, i32* %_M_i.i.i.i.i.i monotonic, align 4
  switch i32 %45, label %_ZN9QtPrivate8RefCount5derefEv.exit.i [
    i32 0, label %if.then.i
    i32 -1, label %ehcleanup29
  ]

_ZN9QtPrivate8RefCount5derefEv.exit.i:            ; preds = %ehcleanup
  %46 = atomicrmw sub i32* %_M_i.i.i.i.i.i, i32 1 seq_cst
  %cmp.i.i.i.i = icmp eq i32 %46, 1
  br i1 %cmp.i.i.i.i, label %_ZN9QtPrivate8RefCount5derefEv.exit.if.then_crit_edge.i, label %ehcleanup29

_ZN9QtPrivate8RefCount5derefEv.exit.if.then_crit_edge.i: ; preds = %_ZN9QtPrivate8RefCount5derefEv.exit.i
  %.pre.i = load %struct.QArrayData*, %struct.QArrayData** %6, align 8, !tbaa !32
  br label %if.then.i

if.then.i:                                        ; preds = %_ZN9QtPrivate8RefCount5derefEv.exit.if.then_crit_edge.i, %ehcleanup
  %47 = phi %struct.QArrayData* [ %.pre.i, %_ZN9QtPrivate8RefCount5derefEv.exit.if.then_crit_edge.i ], [ %44, %ehcleanup ]
  call void @_ZN10QArrayData10deallocateEPS_mm(%struct.QArrayData* %47, i64 1, i64 8) #8
  br label %ehcleanup29

ehcleanup29:                                      ; preds = %if.then.i, %_ZN9QtPrivate8RefCount5derefEv.exit.i, %ehcleanup, %lpad2
  %ehselector.slot.1 = phi i32 [ %11, %lpad2 ], [ %ehselector.slot.0, %ehcleanup ], [ %ehselector.slot.0, %_ZN9QtPrivate8RefCount5derefEv.exit.i ], [ %ehselector.slot.0, %if.then.i ]
  %exn.slot.1 = phi i8* [ %10, %lpad2 ], [ %exn.slot.0, %ehcleanup ], [ %exn.slot.0, %_ZN9QtPrivate8RefCount5derefEv.exit.i ], [ %exn.slot.0, %if.then.i ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %4) #8
  %48 = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 0, i32 0
  call void @_ZN7QObjectD2Ev(%class.QObject* %48) #8
  %lpad.val = insertvalue { i8*, i32 } undef, i8* %exn.slot.1, 0
  %lpad.val31 = insertvalue { i8*, i32 } %lpad.val, i32 %ehselector.slot.1, 1
  resume { i8*, i32 } %lpad.val31
}

declare dso_local void @_ZN15QPlatformCursorC2Ev(%class.QPlatformCursor*) unnamed_addr #1

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

declare dso_local void @_Z7qgetenvPKc(%class.QByteArray* sret, i8*) local_unnamed_addr #1

declare dso_local i32 @_ZNK10QByteArray5toIntEPbi(%class.QByteArray*, i8*, i32) local_unnamed_addr #1

; Function Attrs: nobuiltin
declare dso_local noalias nonnull i8* @_Znwm(i64) local_unnamed_addr #3

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(i8*) local_unnamed_addr #4

; Function Attrs: uwtable
define dso_local void @_ZN9QFbCursor9setCursorEN2Qt11CursorShapeE(%class.QFbCursor* nocapture readonly %this, i32 %shape) local_unnamed_addr #0 align 2 {
entry:
  %mCursorImage = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 7, !intel-tbaa !17
  %0 = load %class.QPlatformCursorImage*, %class.QPlatformCursorImage** %mCursorImage, align 8, !tbaa !17
  tail call void @_ZN20QPlatformCursorImage3setEN2Qt11CursorShapeE(%class.QPlatformCursorImage* %0, i32 %shape)
  ret void
}

declare dso_local void @_ZN19QInputDeviceManager17deviceListChangedENS_10DeviceTypeE(%class.QInputDeviceManager*, i32) #1

; Function Attrs: nounwind
declare dso_local void @_ZN11QMetaObject10ConnectionD1Ev(%"class.QMetaObject::Connection"*) unnamed_addr #5

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

; Function Attrs: nounwind
declare dso_local void @_ZN7QObjectD2Ev(%class.QObject*) unnamed_addr #5

; Function Attrs: nounwind uwtable
define dso_local void @_ZN9QFbCursorD2Ev(%class.QFbCursor* %this) unnamed_addr #6 align 2 {
entry:
  %0 = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [25 x i8*] }, { [25 x i8*] }* @_ZTV9QFbCursor, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %0, align 8, !tbaa !24
  %mDeviceListener = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 8, !intel-tbaa !31
  %1 = load %class.QFbCursorDeviceListener*, %class.QFbCursorDeviceListener** %mDeviceListener, align 8, !tbaa !31
  %isnull = icmp eq %class.QFbCursorDeviceListener* %1, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:                                   ; preds = %entry
  %2 = bitcast %class.QFbCursorDeviceListener* %1 to void (%class.QFbCursorDeviceListener*)***
  %vtable = load void (%class.QFbCursorDeviceListener*)**, void (%class.QFbCursorDeviceListener*)*** %2, align 8, !tbaa !24
  %vfn = getelementptr inbounds void (%class.QFbCursorDeviceListener*)*, void (%class.QFbCursorDeviceListener*)** %vtable, i64 4
  %3 = load void (%class.QFbCursorDeviceListener*)*, void (%class.QFbCursorDeviceListener*)** %vfn, align 8
  tail call void %3(%class.QFbCursorDeviceListener* nonnull %1) #8
  br label %delete.end

delete.end:                                       ; preds = %delete.notnull, %entry
  %4 = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 0, i32 0
  tail call void @_ZN7QObjectD2Ev(%class.QObject* %4) #8
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @_ZN9QFbCursorD0Ev(%class.QFbCursor* %this) unnamed_addr #6 align 2 {
entry:
  %0 = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 0, i32 0, i32 0
  store i32 (...)** bitcast (i8** getelementptr inbounds ({ [25 x i8*] }, { [25 x i8*] }* @_ZTV9QFbCursor, i64 0, inrange i32 0, i64 2) to i32 (...)**), i32 (...)*** %0, align 8, !tbaa !24
  %mDeviceListener.i = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 8, !intel-tbaa !31
  %1 = load %class.QFbCursorDeviceListener*, %class.QFbCursorDeviceListener** %mDeviceListener.i, align 8, !tbaa !31
  %isnull.i = icmp eq %class.QFbCursorDeviceListener* %1, null
  br i1 %isnull.i, label %_ZN9QFbCursorD2Ev.exit, label %delete.notnull.i

delete.notnull.i:                                 ; preds = %entry
  %2 = bitcast %class.QFbCursorDeviceListener* %1 to void (%class.QFbCursorDeviceListener*)***
  %vtable.i = load void (%class.QFbCursorDeviceListener*)**, void (%class.QFbCursorDeviceListener*)*** %2, align 8, !tbaa !24
  %vfn.i = getelementptr inbounds void (%class.QFbCursorDeviceListener*)*, void (%class.QFbCursorDeviceListener*)** %vtable.i, i64 4
  %3 = load void (%class.QFbCursorDeviceListener*)*, void (%class.QFbCursorDeviceListener*)** %vfn.i, align 8
  tail call void %3(%class.QFbCursorDeviceListener* nonnull %1) #8
  br label %_ZN9QFbCursorD2Ev.exit

_ZN9QFbCursorD2Ev.exit:                           ; preds = %entry, %delete.notnull.i
  %4 = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 0, i32 0
  tail call void @_ZN7QObjectD2Ev(%class.QObject* %4) #8
  %5 = bitcast %class.QFbCursor* %this to i8*
  tail call void @_ZdlPv(i8* %5) #10
  ret void
}


declare dso_local { i64, i64 } @_ZNK6QImage4rectEv(%class.QImage*) local_unnamed_addr #1

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i64 @_ZNK9QFbCursor3posEv(%class.QFbCursor* nocapture readonly %this) unnamed_addr #7 align 2 {
entry:
  %retval.sroa.0.0..sroa_idx = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 9
  %retval.sroa.0.0..sroa_cast = bitcast %class.QPoint* %retval.sroa.0.0..sroa_idx to i64*
  %retval.sroa.0.0.copyload = load i64, i64* %retval.sroa.0.0..sroa_cast, align 8
  ret i64 %retval.sroa.0.0.copyload
}


declare dso_local void @_ZN19QInputDeviceManager12setCursorPosERK6QPoint(%class.QInputDeviceManager*, %class.QPoint* dereferenceable(8)) local_unnamed_addr #1

; Function Attrs: nounwind
declare dso_local zeroext i1 @_ZNK5QRect10intersectsERKS_(%class.QRect*, %class.QRect* dereferenceable(16)) local_unnamed_addr #5


; Function Attrs: uwtable
define dso_local { i64, i64 } @_ZN9QFbCursor10drawCursorER8QPainter(%class.QFbCursor* nocapture %this, %class.QPainter* dereferenceable(8) %painter) unnamed_addr #0 align 2 personality i32 (...)* @__gxx_personality_v0 {
entry:
  %ref.tmp.i = alloca %class.QRectF, align 8
  %ref.tmp2.i = alloca %class.QRectF, align 8
  %ref.tmp6 = alloca { i64, i64 }, align 8
  %tmpcast23 = bitcast { i64, i64 }* %ref.tmp6 to %class.QRect*
  %ref.tmp9 = alloca { i64, i64 }, align 8
  %tmpcast24 = bitcast { i64, i64 }* %ref.tmp9 to %class.QRect*
  %mVisible = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 1, !intel-tbaa !7
  %0 = load i8, i8* %mVisible, align 8, !tbaa !7, !range !55
  %tobool = icmp eq i8 %0, 0
  br i1 %tobool, label %return, label %if.end

if.end:                                           ; preds = %entry
  %mDirty = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 5, !intel-tbaa !29
  store i8 0, i8* %mDirty, align 8, !tbaa !29
  %mCurrentRect = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 3, !intel-tbaa !73
  %x2.i25 = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 3, i32 2, !intel-tbaa !58
  %1 = load i32, i32* %x2.i25, align 4, !tbaa !58
  %x1.i26 = getelementptr inbounds %class.QRect, %class.QRect* %mCurrentRect, i64 0, i32 0, !intel-tbaa !56
  %2 = load i32, i32* %x1.i26, align 4, !tbaa !26
  %sub.i = add nsw i32 %2, -1
  %cmp.i = icmp eq i32 %1, %sub.i
  br i1 %cmp.i, label %_ZNK5QRect6isNullEv.exit, label %if.end3

_ZNK5QRect6isNullEv.exit:                         ; preds = %if.end
  %y2.i27 = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 3, i32 3, !intel-tbaa !59
  %3 = load i32, i32* %y2.i27, align 4, !tbaa !59
  %y1.i28 = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 3, i32 1, !intel-tbaa !57
  %4 = load i32, i32* %y1.i28, align 4, !tbaa !57
  %sub2.i = add nsw i32 %4, -1
  %cmp3.i = icmp eq i32 %3, %sub2.i
  br i1 %cmp3.i, label %return, label %if.end3

if.end3:                                          ; preds = %if.end, %_ZNK5QRect6isNullEv.exit
  %mScreen = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 2, !intel-tbaa !16
  %5 = load %class.QFbScreen*, %class.QFbScreen** %mScreen, align 8, !tbaa !16
  %6 = bitcast %class.QFbScreen* %5 to { i64, i64 } (%class.QFbScreen*)***
  %vtable = load { i64, i64 } (%class.QFbScreen*)**, { i64, i64 } (%class.QFbScreen*)*** %6, align 8, !tbaa !24
  %vfn = getelementptr inbounds { i64, i64 } (%class.QFbScreen*)*, { i64, i64 } (%class.QFbScreen*)** %vtable, i64 13
  %7 = load { i64, i64 } (%class.QFbScreen*)*, { i64, i64 } (%class.QFbScreen*)** %vfn, align 8
  %call4 = tail call { i64, i64 } %7(%class.QFbScreen* %5)
  %8 = extractvalue { i64, i64 } %call4, 0
  %ref.tmp.sroa.0.sroa.4.0.extract.shift = lshr i64 %8, 32
  %ref.tmp.sroa.0.sroa.4.0.extract.trunc = trunc i64 %ref.tmp.sroa.0.sroa.4.0.extract.shift to i32
  %mScreenOffset.sroa.0.0.extract.trunc = trunc i64 %8 to i32
  %9 = bitcast { i64, i64 }* %ref.tmp6 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %9) #8
  %10 = load i32, i32* %x1.i26, align 4, !tbaa !26
  %add.i = add nsw i32 %10, %mScreenOffset.sroa.0.0.extract.trunc
  %y1.i41 = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 3, i32 1, !intel-tbaa !57
  %11 = load i32, i32* %y1.i41, align 4, !tbaa !57
  %add4.i = add nsw i32 %11, %ref.tmp.sroa.0.sroa.4.0.extract.trunc
  %12 = load i32, i32* %x2.i25, align 4, !tbaa !58
  %add10.i = add nsw i32 %12, %mScreenOffset.sroa.0.0.extract.trunc
  %y2.i43 = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 3, i32 3, !intel-tbaa !59
  %13 = load i32, i32* %y2.i43, align 4, !tbaa !59
  %add13.i = add nsw i32 %13, %ref.tmp.sroa.0.sroa.4.0.extract.trunc
  %retval.sroa.2.0.insert.ext.i44 = zext i32 %add4.i to i64
  %retval.sroa.2.0.insert.shift.i45 = shl nuw i64 %retval.sroa.2.0.insert.ext.i44, 32
  %retval.sroa.0.0.insert.ext.i46 = zext i32 %add.i to i64
  %retval.sroa.0.0.insert.insert.i47 = or i64 %retval.sroa.2.0.insert.shift.i45, %retval.sroa.0.0.insert.ext.i46
  %retval.sroa.5.8.insert.ext.i = zext i32 %add13.i to i64
  %retval.sroa.5.8.insert.shift.i = shl nuw i64 %retval.sroa.5.8.insert.ext.i, 32
  %retval.sroa.3.8.insert.ext.i = zext i32 %add10.i to i64
  %retval.sroa.3.8.insert.insert.i = or i64 %retval.sroa.5.8.insert.shift.i, %retval.sroa.3.8.insert.ext.i
  %14 = getelementptr inbounds { i64, i64 }, { i64, i64 }* %ref.tmp6, i64 0, i32 0
  store i64 %retval.sroa.0.0.insert.insert.i47, i64* %14, align 8
  %15 = getelementptr inbounds { i64, i64 }, { i64, i64 }* %ref.tmp6, i64 0, i32 1
  store i64 %retval.sroa.3.8.insert.insert.i, i64* %15, align 8
  %16 = bitcast { i64, i64 }* %ref.tmp9 to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %16) #8
  %17 = load %class.QFbScreen*, %class.QFbScreen** %mScreen, align 8, !tbaa !16
  %18 = bitcast %class.QFbScreen* %17 to { i64, i64 } (%class.QFbScreen*)***
  %vtable11 = load { i64, i64 } (%class.QFbScreen*)**, { i64, i64 } (%class.QFbScreen*)*** %18, align 8, !tbaa !24
  %vfn12 = getelementptr inbounds { i64, i64 } (%class.QFbScreen*)*, { i64, i64 } (%class.QFbScreen*)** %vtable11, i64 13
  %19 = load { i64, i64 } (%class.QFbScreen*)*, { i64, i64 } (%class.QFbScreen*)** %vfn12, align 8
  %call13 = tail call { i64, i64 } %19(%class.QFbScreen* %17)
  %20 = getelementptr inbounds { i64, i64 }, { i64, i64 }* %ref.tmp9, i64 0, i32 0
  %21 = extractvalue { i64, i64 } %call13, 0
  store i64 %21, i64* %20, align 8
  %22 = getelementptr inbounds { i64, i64 }, { i64, i64 }* %ref.tmp9, i64 0, i32 1
  %23 = extractvalue { i64, i64 } %call13, 1
  store i64 %23, i64* %22, align 8
  %call14 = call zeroext i1 @_ZNK5QRect10intersectsERKS_(%class.QRect* nonnull %tmpcast23, %class.QRect* nonnull dereferenceable(16) %tmpcast24) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %16) #8
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %9) #8
  br i1 %call14, label %if.end16, label %return

if.end16:                                         ; preds = %if.end3
  %mPrevRect = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 4, !intel-tbaa !74
  %24 = bitcast %class.QRect* %mPrevRect to i8*
  %25 = bitcast %class.QRect* %mCurrentRect to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 %24, i8* nonnull align 4 %25, i64 16, i1 false), !tbaa.struct !75
  %mCursorImage = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 7, !intel-tbaa !17
  %26 = load %class.QPlatformCursorImage*, %class.QPlatformCursorImage** %mCursorImage, align 8, !tbaa !17
  %cursorImage.i = getelementptr inbounds %class.QPlatformCursorImage, %class.QPlatformCursorImage* %26, i64 0, i32 0, !intel-tbaa !18
  %27 = bitcast %class.QRectF* %ref.tmp.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %27) #8
  %xp.i.i = getelementptr inbounds %class.QRectF, %class.QRectF* %ref.tmp.i, i64 0, i32 0, !intel-tbaa !76
  %x1.i.i.i = getelementptr inbounds %class.QRect, %class.QRect* %mPrevRect, i64 0, i32 0, !intel-tbaa !56
  %28 = load i32, i32* %x1.i.i.i, align 4, !tbaa !28
  %conv.i.i = sitofp i32 %28 to double
  store double %conv.i.i, double* %xp.i.i, align 8, !tbaa !76
  %yp.i.i = getelementptr inbounds %class.QRectF, %class.QRectF* %ref.tmp.i, i64 0, i32 1, !intel-tbaa !78
  %y1.i14.i.i = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 4, i32 1, !intel-tbaa !79
  %29 = load i32, i32* %y1.i14.i.i, align 4, !tbaa !79
  %conv3.i.i = sitofp i32 %29 to double
  store double %conv3.i.i, double* %yp.i.i, align 8, !tbaa !78
  %w.i.i = getelementptr inbounds %class.QRectF, %class.QRectF* %ref.tmp.i, i64 0, i32 2, !intel-tbaa !80
  %x2.i.i.i = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 4, i32 2
  %30 = load i32, i32* %x2.i.i.i, align 4, !tbaa !81
  %sub.i12.i.i = sub i32 1, %28
  %add.i13.i.i = add i32 %sub.i12.i.i, %30
  %conv5.i.i = sitofp i32 %add.i13.i.i to double
  store double %conv5.i.i, double* %w.i.i, align 8, !tbaa !80
  %h.i.i = getelementptr inbounds %class.QRectF, %class.QRectF* %ref.tmp.i, i64 0, i32 3, !intel-tbaa !82
  %y2.i.i.i = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 4, i32 3, !intel-tbaa !83
  %31 = load i32, i32* %y2.i.i.i, align 4, !tbaa !83
  %sub.i.i.i = sub i32 1, %29
  %add.i.i.i = add i32 %sub.i.i.i, %31
  %conv7.i.i = sitofp i32 %add.i.i.i to double
  store double %conv7.i.i, double* %h.i.i, align 8, !tbaa !82
  %32 = bitcast %class.QRectF* %ref.tmp2.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %32) #8
  %call.i = call i32 @_ZNK6QImage5widthEv(%class.QImage* nonnull %cursorImage.i)
  %conv.i = sitofp i32 %call.i to double
  %call3.i = call i32 @_ZNK6QImage6heightEv(%class.QImage* nonnull %cursorImage.i)
  %conv4.i = sitofp i32 %call3.i to double
  %w.i9.i = getelementptr inbounds %class.QRectF, %class.QRectF* %ref.tmp2.i, i64 0, i32 2, !intel-tbaa !80
  call void @llvm.memset.p0i8.i64(i8* nonnull align 8 %32, i8 0, i64 16, i1 false)
  store double %conv.i, double* %w.i9.i, align 8, !tbaa !80
  %h.i10.i = getelementptr inbounds %class.QRectF, %class.QRectF* %ref.tmp2.i, i64 0, i32 3, !intel-tbaa !82
  store double %conv4.i, double* %h.i10.i, align 8, !tbaa !82
  call void @_ZN8QPainter9drawImageERK6QRectFRK6QImageS2_6QFlagsIN2Qt19ImageConversionFlagEE(%class.QPainter* nonnull %painter, %class.QRectF* nonnull dereferenceable(32) %ref.tmp.i, %class.QImage* nonnull dereferenceable(32) %cursorImage.i, %class.QRectF* nonnull dereferenceable(32) %ref.tmp2.i, i32 0)
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %32) #8
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %27) #8
  %mOnScreen = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 6, !intel-tbaa !30
  store i8 1, i8* %mOnScreen, align 1, !tbaa !30
  %retval.sroa.0.0..sroa_cast = bitcast %class.QRect* %mPrevRect to i64*
  %retval.sroa.0.0.copyload = load i64, i64* %retval.sroa.0.0..sroa_cast, align 8
  %retval.sroa.8.0..sroa_cast = bitcast i32* %x2.i.i.i to i64*
  %retval.sroa.8.0.copyload = load i64, i64* %retval.sroa.8.0..sroa_cast, align 8
  %phitmp = and i64 %retval.sroa.0.0.copyload, -4294967296
  %phitmp67 = and i64 %retval.sroa.0.0.copyload, 4294967295
  br label %return

return:                                           ; preds = %if.end16, %if.end3, %_ZNK5QRect6isNullEv.exit, %entry
  %retval.sroa.8.1 = phi i64 [ -1, %entry ], [ -1, %_ZNK5QRect6isNullEv.exit ], [ %retval.sroa.8.0.copyload, %if.end16 ], [ -1, %if.end3 ]
  %retval.sroa.0.sroa.0.1 = phi i64 [ 0, %entry ], [ 0, %_ZNK5QRect6isNullEv.exit ], [ %phitmp67, %if.end16 ], [ 0, %if.end3 ]
  %retval.sroa.0.sroa.5.1 = phi i64 [ 0, %entry ], [ 0, %_ZNK5QRect6isNullEv.exit ], [ %phitmp, %if.end16 ], [ 0, %if.end3 ]
  %retval.sroa.0.sroa.0.0.insert.insert = or i64 %retval.sroa.0.sroa.5.1, %retval.sroa.0.sroa.0.1
  %.fca.0.insert = insertvalue { i64, i64 } undef, i64 %retval.sroa.0.sroa.0.0.insert.insert, 0
  %.fca.1.insert = insertvalue { i64, i64 } %.fca.0.insert, i64 %retval.sroa.8.1, 1
  ret { i64, i64 } %.fca.1.insert
}

; Function Attrs: nounwind uwtable
define dso_local { i64, i64 } @_ZN9QFbCursor9dirtyRectEv(%class.QFbCursor* nocapture %this) local_unnamed_addr #6 align 2 {
entry:
  %mOnScreen = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 6, !intel-tbaa !30
  %0 = load i8, i8* %mOnScreen, align 1, !tbaa !30, !range !55
  %tobool = icmp eq i8 %0, 0
  br i1 %tobool, label %return, label %if.then

if.then:                                          ; preds = %entry
  store i8 0, i8* %mOnScreen, align 1, !tbaa !30
  %retval.sroa.0.0..sroa_idx = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 4
  %retval.sroa.0.0..sroa_cast = bitcast %class.QRect* %retval.sroa.0.0..sroa_idx to i64*
  %retval.sroa.0.0.copyload = load i64, i64* %retval.sroa.0.0..sroa_cast, align 8
  %retval.sroa.0.sroa.3.0.extract.shift = and i64 %retval.sroa.0.0.copyload, -4294967296
  %retval.sroa.4.0..sroa_idx3 = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 4, i32 2
  %retval.sroa.4.0..sroa_cast = bitcast i32* %retval.sroa.4.0..sroa_idx3 to i64*
  %retval.sroa.4.0.copyload = load i64, i64* %retval.sroa.4.0..sroa_cast, align 8
  %phitmp = and i64 %retval.sroa.0.0.copyload, 4294967295
  br label %return

return:                                           ; preds = %entry, %if.then
  %retval.sroa.0.sroa.3.0 = phi i64 [ %retval.sroa.0.sroa.3.0.extract.shift, %if.then ], [ 0, %entry ]
  %retval.sroa.0.sroa.0.0 = phi i64 [ %phitmp, %if.then ], [ 0, %entry ]
  %retval.sroa.4.0 = phi i64 [ %retval.sroa.4.0.copyload, %if.then ], [ -1, %entry ]
  %retval.sroa.0.sroa.0.0.insert.insert = or i64 %retval.sroa.0.sroa.0.0, %retval.sroa.0.sroa.3.0
  %.fca.0.insert = insertvalue { i64, i64 } undef, i64 %retval.sroa.0.sroa.0.0.insert.insert, 0
  %.fca.1.insert = insertvalue { i64, i64 } %.fca.0.insert, i64 %retval.sroa.4.0, 1
  ret { i64, i64 } %.fca.1.insert
}

declare dso_local void @_ZN20QPlatformCursorImage3setEN2Qt11CursorShapeE(%class.QPlatformCursorImage*, i32) local_unnamed_addr #1

; Function Attrs: uwtable
define dso_local void @_ZN9QFbCursor9setCursorERK6QImageii(%class.QFbCursor* nocapture readonly %this, %class.QImage* dereferenceable(32) %image, i32 %hotx, i32 %hoty) local_unnamed_addr #0 align 2 {
entry:
  %mCursorImage = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 7, !intel-tbaa !17
  %0 = load %class.QPlatformCursorImage*, %class.QPlatformCursorImage** %mCursorImage, align 8, !tbaa !17
  tail call void @_ZN20QPlatformCursorImage3setERK6QImageii(%class.QPlatformCursorImage* %0, %class.QImage* nonnull dereferenceable(32) %image, i32 %hotx, i32 %hoty)
  ret void
}

declare dso_local void @_ZN20QPlatformCursorImage3setERK6QImageii(%class.QPlatformCursorImage*, %class.QImage* dereferenceable(32), i32, i32) local_unnamed_addr #1

; Function Attrs: uwtable
define dso_local void @_ZN9QFbCursor9setCursorEPKhS1_iiii(%class.QFbCursor* nocapture readonly %this, i8* %data, i8* %mask, i32 %width, i32 %height, i32 %hotX, i32 %hotY) local_unnamed_addr #0 align 2 {
entry:
  %mCursorImage = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 7, !intel-tbaa !17
  %0 = load %class.QPlatformCursorImage*, %class.QPlatformCursorImage** %mCursorImage, align 8, !tbaa !17
  tail call void @_ZN20QPlatformCursorImage3setEPKhS1_iiii(%class.QPlatformCursorImage* %0, i8* %data, i8* %mask, i32 %width, i32 %height, i32 %hotX, i32 %hotY)
  ret void
}

declare dso_local void @_ZN20QPlatformCursorImage3setEPKhS1_iiii(%class.QPlatformCursorImage*, i8*, i8*, i32, i32, i32, i32) local_unnamed_addr #1


; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #2

declare dso_local i32 @_ZNK7QCursor5shapeEv(%class.QCursor*) local_unnamed_addr #1

declare dso_local i64 @_ZNK7QCursor7hotSpotEv(%class.QCursor*) local_unnamed_addr #1

declare dso_local void @_ZNK7QCursor6pixmapEv(%class.QPixmap* sret, %class.QCursor*) local_unnamed_addr #1

declare dso_local void @_ZNK7QPixmap7toImageEv(%class.QImage* sret, %class.QPixmap*) local_unnamed_addr #1

; Function Attrs: nounwind
declare dso_local void @_ZN6QImageD1Ev(%class.QImage*) unnamed_addr #5

; Function Attrs: nounwind
declare dso_local void @_ZN7QPixmapD1Ev(%class.QPixmap*) unnamed_addr #5

; Function Attrs: uwtable
define dso_local void @_ZN9QFbCursor8setDirtyEv(%class.QFbCursor* nocapture %this) unnamed_addr #0 align 2 {
entry:
  %mVisible = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 1, !intel-tbaa !7
  %0 = load i8, i8* %mVisible, align 8, !tbaa !7, !range !55
  %tobool = icmp eq i8 %0, 0
  br i1 %tobool, label %if.end5, label %if.end

if.end:                                           ; preds = %entry
  %mDirty = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 5, !intel-tbaa !29
  %1 = load i8, i8* %mDirty, align 8, !tbaa !29, !range !55
  %tobool2 = icmp eq i8 %1, 0
  br i1 %tobool2, label %if.then3, label %if.end5

if.then3:                                         ; preds = %if.end
  store i8 1, i8* %mDirty, align 8, !tbaa !29
  %mScreen = getelementptr inbounds %class.QFbCursor, %class.QFbCursor* %this, i64 0, i32 2, !intel-tbaa !16
  %2 = load %class.QFbScreen*, %class.QFbScreen** %mScreen, align 8, !tbaa !16
  tail call void @_ZN9QFbScreen14scheduleUpdateEv(%class.QFbScreen* %2)
  br label %if.end5

if.end5:                                          ; preds = %if.end, %entry, %if.then3
  ret void
}

declare dso_local void @_ZN9QFbScreen14scheduleUpdateEv(%class.QFbScreen*) local_unnamed_addr #1

; Function Attrs: nounwind
declare dso_local void @_ZN6QImageC1Ev(%class.QImage*) unnamed_addr #5

declare dso_local void @_ZN7QObjectC2EPS_(%class.QObject*, %class.QObject*) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local void @_ZN10QArrayData10deallocateEPS_mm(%struct.QArrayData*, i64, i64) local_unnamed_addr #5

declare dso_local void @_ZN8QPainter9drawImageERK6QRectFRK6QImageS2_6QFlagsIN2Qt19ImageConversionFlagEE(%class.QPainter*, %class.QRectF* dereferenceable(32), %class.QImage* dereferenceable(32), %class.QRectF* dereferenceable(32), i32) local_unnamed_addr #1

declare dso_local i32 @_ZNK6QImage5widthEv(%class.QImage*) local_unnamed_addr #1

declare dso_local i32 @_ZNK6QImage6heightEv(%class.QImage*) local_unnamed_addr #1

declare dso_local void @_ZN7QObject11connectImplEPKS_PPvS1_S3_PN9QtPrivate15QSlotObjectBaseEN2Qt14ConnectionTypeEPKiPK11QMetaObject(%"class.QMetaObject::Connection"* sret, %class.QObject*, i8**, %class.QObject*, i8**, %"class.QtPrivate::QSlotObjectBase"*, i32, i32*, %struct.QMetaObject*) local_unnamed_addr #1

; Function Attrs: uwtable
define linkonce_odr dso_local void @_ZN9QtPrivate11QSlotObjectIM23QFbCursorDeviceListenerFvN19QInputDeviceManager10DeviceTypeEENS_4ListIIS3_EEEvE4implEiPNS_15QSlotObjectBaseEP7QObjectPPvPb(i32 %which, %"class.QtPrivate::QSlotObjectBase"* %this_, %class.QObject* %r, i8** %a, i8* %ret) #0 comdat align 2 {
entry:
  switch i32 %which, label %sw.epilog [
    i32 0, label %sw.bb
    i32 1, label %sw.bb1
    i32 2, label %sw.bb2
  ]

sw.bb:                                            ; preds = %entry
  %isnull = icmp eq %"class.QtPrivate::QSlotObjectBase"* %this_, null
  br i1 %isnull, label %sw.epilog, label %delete.notnull

delete.notnull:                                   ; preds = %sw.bb
  %0 = bitcast %"class.QtPrivate::QSlotObjectBase"* %this_ to i8*
  tail call void @_ZdlPv(i8* %0) #10
  br label %sw.epilog

sw.bb1:                                           ; preds = %entry
  %function = getelementptr inbounds %"class.QtPrivate::QSlotObjectBase", %"class.QtPrivate::QSlotObjectBase"* %this_, i64 1
  %.elt14 = bitcast %"class.QtPrivate::QSlotObjectBase"* %function to i64*
  %.unpack15 = load i64, i64* %.elt14, align 8, !tbaa !53
  %.elt16 = getelementptr inbounds %"class.QtPrivate::QSlotObjectBase", %"class.QtPrivate::QSlotObjectBase"* %this_, i64 1, i32 1
  %1 = bitcast void (i32, %"class.QtPrivate::QSlotObjectBase"*, %class.QObject*, i8**, i8*)** %.elt16 to i64*
  %.unpack17 = load i64, i64* %1, align 8, !tbaa !53
  %2 = bitcast %class.QObject* %r to i8*
  %3 = getelementptr inbounds i8, i8* %2, i64 %.unpack17
  %this.adjusted.i.i = bitcast i8* %3 to %class.QFbCursorDeviceListener*
  %4 = and i64 %.unpack15, 1
  %memptr.isvirtual.i.i = icmp eq i64 %4, 0
  br i1 %memptr.isvirtual.i.i, label %memptr.nonvirtual.i.i, label %memptr.virtual.i.i

memptr.virtual.i.i:                               ; preds = %sw.bb1
  %5 = bitcast i8* %3 to i8**
  %vtable.i.i = load i8*, i8** %5, align 8, !tbaa !24
  %6 = add i64 %.unpack15, -1
  %7 = getelementptr i8, i8* %vtable.i.i, i64 %6
  %8 = bitcast i8* %7 to void (%class.QFbCursorDeviceListener*, i32)**
  %memptr.virtualfn.i.i = load void (%class.QFbCursorDeviceListener*, i32)*, void (%class.QFbCursorDeviceListener*, i32)** %8, align 8
  br label %_ZN9QtPrivate15FunctionPointerIM23QFbCursorDeviceListenerFvN19QInputDeviceManager10DeviceTypeEEE4callINS_4ListIIS3_EEEvEEvS5_PS1_PPv.exit

memptr.nonvirtual.i.i:                            ; preds = %sw.bb1
  %memptr.nonvirtualfn.i.i = inttoptr i64 %.unpack15 to void (%class.QFbCursorDeviceListener*, i32)*
  br label %_ZN9QtPrivate15FunctionPointerIM23QFbCursorDeviceListenerFvN19QInputDeviceManager10DeviceTypeEEE4callINS_4ListIIS3_EEEvEEvS5_PS1_PPv.exit

_ZN9QtPrivate15FunctionPointerIM23QFbCursorDeviceListenerFvN19QInputDeviceManager10DeviceTypeEEE4callINS_4ListIIS3_EEEvEEvS5_PS1_PPv.exit: ; preds = %memptr.virtual.i.i, %memptr.nonvirtual.i.i
  %9 = phi void (%class.QFbCursorDeviceListener*, i32)* [ %memptr.virtualfn.i.i, %memptr.virtual.i.i ], [ %memptr.nonvirtualfn.i.i, %memptr.nonvirtual.i.i ]
  %arrayidx.i.i = getelementptr inbounds i8*, i8** %a, i64 1
  %10 = bitcast i8** %arrayidx.i.i to i32**
  %11 = load i32*, i32** %10, align 8, !tbaa !84
  %12 = load i32, i32* %11, align 4, !tbaa !86
  tail call void %9(%class.QFbCursorDeviceListener* %this.adjusted.i.i, i32 %12)
  br label %sw.epilog

sw.bb2:                                           ; preds = %entry
  %.elt = bitcast i8** %a to i64*
  %.unpack = load i64, i64* %.elt, align 8, !tbaa !43
  %.elt8 = getelementptr inbounds i8*, i8** %a, i64 1
  %13 = bitcast i8** %.elt8 to i64*
  %.unpack9 = load i64, i64* %13, align 8, !tbaa !43
  %function3 = getelementptr inbounds %"class.QtPrivate::QSlotObjectBase", %"class.QtPrivate::QSlotObjectBase"* %this_, i64 1
  %.elt10 = bitcast %"class.QtPrivate::QSlotObjectBase"* %function3 to i64*
  %.unpack11 = load i64, i64* %.elt10, align 8, !tbaa !53
  %.elt12 = getelementptr inbounds %"class.QtPrivate::QSlotObjectBase", %"class.QtPrivate::QSlotObjectBase"* %this_, i64 1, i32 1
  %14 = bitcast void (i32, %"class.QtPrivate::QSlotObjectBase"*, %class.QObject*, i8**, i8*)** %.elt12 to i64*
  %.unpack13 = load i64, i64* %14, align 8, !tbaa !53
  %cmp.ptr = icmp eq i64 %.unpack, %.unpack11
  %cmp.ptr.null = icmp eq i64 %.unpack, 0
  %cmp.adj = icmp eq i64 %.unpack9, %.unpack13
  %15 = or i1 %cmp.ptr.null, %cmp.adj
  %memptr.eq = and i1 %cmp.ptr, %15
  %frombool = zext i1 %memptr.eq to i8
  store i8 %frombool, i8* %ret, align 1, !tbaa !88
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.bb, %delete.notnull, %entry, %sw.bb2, %_ZN9QtPrivate15FunctionPointerIM23QFbCursorDeviceListenerFvN19QInputDeviceManager10DeviceTypeEEE4callINS_4ListIIS3_EEEvEEvS5_PS1_PPv.exit
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #2

attributes #0 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nobuiltin "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nobuiltin nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #8 = { nounwind }
attributes #9 = { builtin }
attributes #10 = { builtin nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57c33ad4c535ae8b183eddbaecd43589a368ac99) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm f7aeee7c72dd637d050296f8a574f7ca3ab83646)"}
!2 = !{!3, !4, i64 16}
!3 = !{!"struct@_ZTS23QFbCursorDeviceListener", !4, i64 16}
!4 = !{!"pointer@_ZTSP9QFbCursor", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !9, i64 16}
!8 = !{!"struct@_ZTS9QFbCursor", !9, i64 16, !10, i64 24, !11, i64 32, !11, i64 48, !9, i64 64, !9, i64 65, !13, i64 72, !14, i64 80, !15, i64 88}
!9 = !{!"bool", !5, i64 0}
!10 = !{!"pointer@_ZTSP9QFbScreen", !5, i64 0}
!11 = !{!"struct@_ZTS5QRect", !12, i64 0, !12, i64 4, !12, i64 8, !12, i64 12}
!12 = !{!"int", !5, i64 0}
!13 = !{!"pointer@_ZTSP20QPlatformCursorImage", !5, i64 0}
!14 = !{!"pointer@_ZTSP23QFbCursorDeviceListener", !5, i64 0}
!15 = !{!"struct@_ZTS6QPoint", !12, i64 0, !12, i64 4}
!16 = !{!8, !10, i64 24}
!17 = !{!8, !13, i64 72}
!18 = !{!19, !20, i64 0}
!19 = !{!"struct@_ZTS20QPlatformCursorImage", !20, i64 0, !15, i64 32}
!20 = !{!"struct@_ZTS6QImage", !21, i64 24}
!21 = !{!"pointer@_ZTSP10QImageData", !5, i64 0}
!22 = !{!8, !12, i64 88}
!23 = !{!8, !12, i64 92}
!24 = !{!25, !25, i64 0}
!25 = !{!"vtable pointer", !6, i64 0}
!26 = !{!8, !12, i64 32}
!27 = !{!12, !12, i64 0}
!28 = !{!8, !12, i64 48}
!29 = !{!8, !9, i64 64}
!30 = !{!8, !9, i64 65}
!31 = !{!8, !14, i64 80}
!32 = !{!33, !34, i64 0}
!33 = !{!"struct@_ZTS10QByteArray", !34, i64 0}
!34 = !{!"pointer@_ZTSP15QTypedArrayDataIcE", !5, i64 0}
!35 = !{!36, !12, i64 4}
!36 = !{!"struct@_ZTS10QArrayData", !37, i64 0, !12, i64 4, !12, i64 8, !12, i64 11, !40, i64 16}
!37 = !{!"struct@_ZTSN9QtPrivate8RefCountE", !38, i64 0}
!38 = !{!"struct@_ZTS19QBasicAtomicIntegerIiE", !39, i64 0}
!39 = !{!"struct@_ZTSSt6atomicIiE"}
!40 = !{!"long long", !5, i64 0}
!41 = !{!19, !12, i64 32}
!42 = !{!19, !12, i64 36}
!43 = !{!5, !5, i64 0}
!44 = !{!45}
!45 = distinct !{!45, !46, !"_ZN7QObject7connectIM19QInputDeviceManagerFvNS1_10DeviceTypeEEM23QFbCursorDeviceListenerFvS2_EEEN11QMetaObject10ConnectionEPKN9QtPrivate15FunctionPointerIT_E6ObjectESC_PKNSB_IT0_E6ObjectESH_N2Qt14ConnectionTypeE: %agg.result"}
!46 = distinct !{!46, !"_ZN7QObject7connectIM19QInputDeviceManagerFvNS1_10DeviceTypeEEM23QFbCursorDeviceListenerFvS2_EEEN11QMetaObject10ConnectionEPKN9QtPrivate15FunctionPointerIT_E6ObjectESC_PKNSB_IT0_E6ObjectESH_N2Qt14ConnectionTypeE"}
!47 = !{!48, !12, i64 0}
!48 = !{!"struct@_ZTSSt13__atomic_baseIiE", !12, i64 0}
!49 = !{!50, !52, i64 8}
!50 = !{!"struct@_ZTSN9QtPrivate15QSlotObjectBaseE", !51, i64 0, !52, i64 8}
!51 = !{!"struct@_ZTS10QAtomicInt"}
!52 = !{!"pointer@_ZTSPFviPN9QtPrivate15QSlotObjectBaseEP7QObjectPPvPbE", !5, i64 0}
!53 = !{!54, !5, i64 16}
!54 = !{!"struct@_ZTSN9QtPrivate11QSlotObjectIM23QFbCursorDeviceListenerFvN19QInputDeviceManager10DeviceTypeEENS_4ListIIS3_EEEvEE", !5, i64 16}
!55 = !{i8 0, i8 2}
!56 = !{!11, !12, i64 0}
!57 = !{!8, !12, i64 36}
!58 = !{!8, !12, i64 40}
!59 = !{!8, !12, i64 44}
!60 = !{!61, !63, i64 16}
!61 = !{!"struct@_ZTS6QEvent", !62, i64 8, !63, i64 16, !63, i64 18, !63, i64 18, !63, i64 18, !63, i64 18}
!62 = !{!"pointer@_ZTSP13QEventPrivate", !5, i64 0}
!63 = !{!"short", !5, i64 0}
!64 = !{!65, !67, i64 64}
!65 = !{!"struct@_ZTS11QMouseEvent", !66, i64 32, !66, i64 48, !66, i64 64, !68, i64 80, !69, i64 84, !12, i64 88, !70, i64 92}
!66 = !{!"struct@_ZTS7QPointF", !67, i64 0, !67, i64 8}
!67 = !{!"double", !5, i64 0}
!68 = !{!"_ZTSN2Qt11MouseButtonE", !5, i64 0}
!69 = !{!"struct@_ZTS6QFlagsIN2Qt11MouseButtonEE", !12, i64 0}
!70 = !{!"struct@_ZTS9QVector2D", !71, i64 0, !71, i64 4}
!71 = !{!"float", !5, i64 0}
!72 = !{!65, !67, i64 72}
!73 = !{!8, !11, i64 32}
!74 = !{!8, !11, i64 48}
!75 = !{i64 0, i64 4, !27, i64 4, i64 4, !27, i64 8, i64 4, !27, i64 12, i64 4, !27}
!76 = !{!77, !67, i64 0}
!77 = !{!"struct@_ZTS6QRectF", !67, i64 0, !67, i64 8, !67, i64 16, !67, i64 24}
!78 = !{!77, !67, i64 8}
!79 = !{!8, !12, i64 52}
!80 = !{!77, !67, i64 16}
!81 = !{!8, !12, i64 56}
!82 = !{!77, !67, i64 24}
!83 = !{!8, !12, i64 60}
!84 = !{!85, !85, i64 0}
!85 = !{!"pointer@_ZTSPv", !5, i64 0}
!86 = !{!87, !87, i64 0}
!87 = !{!"_ZTSN19QInputDeviceManager10DeviceTypeE", !5, i64 0}
!88 = !{!9, !9, i64 0}
