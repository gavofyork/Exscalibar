#pragma once

#define __GEDDEI_BUILD

#include <QtXml>
#include <QtGui>
#include <Geddei>

#include "inputitem.h"
#include "outputitem.h"

static const double cornerSize = 4.0;
static const double statusHeight = 8.0;
static const double statusMargin = 2.0;
static const double portSize = 8.0;
static const double portLateralMargin = 4.0;

class PauseItem;

class ProcessorItem: public QObject, public QGraphicsItem
{
	Q_OBJECT

	friend class InputItem;
	friend class OutputItem;

public:
	ProcessorItem(Processor* _p, Properties const& _pr = Properties(), QString const& _name = QString::null, QSizeF const& _size = QSizeF(0, 0));
	~ProcessorItem()
	{
		delete m_processor;
	}

	Processor* processor() const { return m_processor; }
	QGraphicsItem* statusBar() const { return m_statusBar; }
	QRectF clientArea() const;

	void adjustSize(QSizeF const& _a);

	Properties const& properties() const { return m_properties; }
	void setProperty(QString const& _key, QVariant const& _value);

	bool connectYourself(ProcessorGroup& _group);
	void typesConfirmed();
	void disconnectYourself();

	void tick();

	static void fromDom(QDomElement& _element, QGraphicsScene* _scene);
	virtual QDomElement saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n = "processor") const;

	virtual QSizeF centreMin() const { return QSizeF(m_processor->width(), m_processor->height()); }

	virtual QRectF boundingRect() const
	{
		return QRectF(QPointF(-10, -10), m_size + QSize(20.f, 20.f));
	}
	virtual void focusInEvent(QFocusEvent* _e);
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);
	virtual bool sceneEvent(QEvent* _e) { return QGraphicsItem::sceneEvent(_e); }
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* _e);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* _e);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* _e);
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* _e);

	enum { Type = UserType + 1 };
	virtual int type() const { return Type; }

	virtual void propertiesChanged(QString const& _newName = QString::null);

protected:
	virtual void rejig(Processor* _old = 0, bool _bootStrap = false);
	virtual Processor* reconstructProcessor();
	virtual Properties completeProperties() const { return m_properties; }

	Properties			m_properties;
	Processor*			m_processor;

private:
	virtual void timerEvent(QTimerEvent*);

	QGraphicsRectItem*	m_statusBar;
	PauseItem*			m_pauseItem;
	QSizeF				m_size;
	int					m_timerId;
	bool				m_resizing;
	QPointF				m_origPosition;
	uint				m_multiplicity;
};

class SubProcessorItem;

class DomProcessorItem: public ProcessorItem
{
public:
	DomProcessorItem(Properties const& _pr = Properties("Latency/Throughput", 0.0), QString const& _name = QString::null, QSizeF const& _size = QSizeF(0, 0));

	DomProcessor* domProcessor() const;

//	enum { Type = UserType + 7 };
//	virtual int type() const { return Type; }

	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);

	virtual QDomElement saveYourself(QDomElement& _root, QDomDocument& _doc, QString const& _n = "domprocessor") const;
	static void fromDom(QDomElement& _element, QGraphicsScene* _scene);

	void reorder() const;

protected:
	virtual void rejig(Processor* _old = 0, bool _bootStrap = false);
	virtual QSizeF centreMin() const;
	virtual Properties completeProperties() const;
	virtual Processor* reconstructProcessor();

private:
	QString composedSubs() const;
	QList<SubProcessorItem*> ordered() const;
};

class SubProcessorItem: public QGraphicsItem
{
	friend class DomProcessorItem;// so it can have its properties populated.

public:
	SubProcessorItem(DomProcessorItem* _dpi, QString const& _type, int _index = 0, Properties const& _pr = Properties());

	QSizeF size() const { return QSizeF(subProcessor()->width(), subProcessor()->height()); }

	uint index() const { return m_index; }
	DomProcessorItem* domProcessorItem() const { return qgraphicsitem_cast<DomProcessorItem*>(parentItem()); }
	DomProcessor* domProcessor() const { return domProcessorItem()->domProcessor(); }
	SubProcessor* subProcessor() const;

	void saveYourself(QDomElement& _root, QDomDocument& _doc) const;
	static void fromDom(QDomElement const& _element, DomProcessorItem* _dpi);

	QString spType() const { return m_type; }

	enum { Type = UserType + 8 };
	virtual int type() const { return Type; }

	Properties const& properties() const { return m_properties; }
	void setProperty(QString const& _key, QVariant const& _value);

private:
	virtual QRectF boundingRect() const
	{
		return QRectF(QPointF(-2, -2), size() + QSizeF(4, 4));
	}
	virtual void paint(QPainter* _p, const QStyleOptionGraphicsItem*, QWidget*);
	virtual void focusInEvent(QFocusEvent* _e);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent* _e);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* _e);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* _e);

	Properties			m_properties;
	QString				m_type;
	uint				m_index;
};
