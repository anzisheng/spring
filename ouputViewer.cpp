#include "ouputViewer.h"
#include "ClassDefined.h"
#include "StaticValue.h"

static ouputViewer* p_outputViewer;

ouputViewer::ouputViewer(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.output_color->setAutoFillBackground(true);
}

void ouputViewer::loopDMXstatus()
{
	//设备号
/*	int device_id = 289;
	int out_put_device_id = StaticValue::GetInstance()->m_device_map[device_id]->output_device_id;
	OutPutDevice* out_put_Device = StaticValue::GetInstance()->m_output_device[out_put_device_id];
	unsigned char	r, g, b;
	r = out_put_Device->m_route_state[0];
	g = out_put_Device->m_route_state[1];
	b = out_put_Device->m_route_state[2];
	QColor bgcolor(r, g, b);
	QPalette pal = ui.output_color->palette();
	pal.setColor(QPalette::Window, bgcolor);
	ui.output_color->setPalette(pal);
	*/
}

ouputViewer::~ouputViewer()
{
}

ouputViewer* ouputViewer::GetOPViewerInstance()
{
	if (p_outputViewer==nullptr)
	{
		p_outputViewer = new ouputViewer();
	}
	return p_outputViewer;
}
