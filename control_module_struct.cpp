//---------------------------------------------------------------------------

#pragma hdrstop

#include "control_module_struct.h"
#include <memory>
//---------------------------------------------------------------------------
#pragma package(smart_init)


namespace amp {

//	failed to do via templates as you still need to do the type casts from string variant.
template<class T> void ReadAttrValueIfExists(T& x, const Xml::Xmlintf::_di_IXMLNode& node, const char* cattr )
{
	if (node->HasAttribute(cattr))
		x = node->Attributes[cattr];
}

void ReadStrAttrIfExists(String& v, const Xml::Xmlintf::_di_IXMLNode& node, const char* cattr )
{
	if (node->HasAttribute(cattr)) {
		AnsiString tmp = node->Attributes[cattr];
		v = tmp.c_str();
	}
}

void ReadBoolAttrIfExists(bool& v, const Xml::Xmlintf::_di_IXMLNode& node, const char* cattr )
{
	if (node->HasAttribute(cattr)) {
		AnsiString tmp = node->Attributes[cattr];
		if (tmp=="1") {
			v = true;
		} else if (tmp=="0") {
				   v = false;
			   } else
					throw Exception("Unexpected boolean value : " + tmp + ". Property: " + cattr );
	}
}

void ReadIntAttrIfExists(int& v, const Xml::Xmlintf::_di_IXMLNode& node, const char* cattr )
{
	if (node->HasAttribute(cattr)) {
		AnsiString tmp = node->Attributes[cattr];
		try {
			v = StrToInt(tmp);
		} catch (EConvertError & e) {
			throw Exception("Unexpected integer value : " + tmp + ". Property: " + cattr );
		}
	}
}

element_with_timeout::element_with_timeout() :
						timeout(0),
						kill_on_timeout(true)
//						,extra_wait_before_kill(0)
{}

void element_with_timeout::read_from_node(const TXMLNode& node)
{
	ReadIntAttrIfExists(timeout, node, "timeout");
}

send_run_element::send_run_element() :
	timestamp(false)
{
}

element_with_index::element_with_index(): ctl_index(0)
{

}

int element_with_index::index()
{
	return ctl_index;
}

void element_with_index::set_index(int i)
{
	ctl_index = i;
}

element_with_retry::element_with_retry()
{
	retries = 0;
	lockwait = 0;
}

void element_with_retry::read_from_node(const TXMLNode& node)
{
	ReadIntAttrIfExists(lockwait, node, "retries");
	ReadIntAttrIfExists(lockwait, node, "lockwait");
}

element_with_time_filter::element_with_time_filter()
{
	enabled = true;
}

void element_with_time_filter::read_from_node(const TXMLNode& node)
{
	ReadBoolAttrIfExists(enabled, node, "enabled");
	ReadStrAttrIfExists(enabled_logic, node, "enabled_logic");
	ReadStrAttrIfExists(days_of_week, node, "daysofweek");
	ReadStrAttrIfExists(start_time, node, "starttime");
	ReadStrAttrIfExists(end_time, node, "endtime");
}

bool   element_with_time_filter::is_ok_time(const TDateTime &t)
{
	if (!start_time.IsEmpty() || !end_time.IsEmpty()) {
		String s = t.FormatString("hh:nn");
		if (!start_time.IsEmpty() && s < start_time || !end_time.IsEmpty() && s > end_time )
			return false;
	}
	return true;
}

bool   element_with_time_filter::is_ok_day(const TDateTime &t)
{
	if (!days_of_week.IsEmpty()) {
		int dow = t.DayOfWeek();
		dow--;
		if (!dow)
			dow = 7; //	transform to Moday to 1 and Sunday to 7
		if (days_of_week.Pos(IntToStr(dow))==0)
			return false;
	}
	return true;
}

bool   element_with_time_filter::is_allowed_by_filter(const TDateTime &t, String& message)
{
	message = "";
	if (!enabled) {
		message = "enabled=0";
	}
	if (!is_ok_time(t)) {
		message = "time filter ("+start_time +"-"+end_time+")";
	}
	if (message.IsEmpty() && !is_ok_day(t)) {
		message = "day of week filter ("+days_of_week+")";
	}
	return message.IsEmpty();
}

run_item_data::run_item_data():wait(0)
{

}

void run_item_data::read_from_node(const TXMLNode& node)
{
	element_with_timeout::read_from_node(node);
	element_with_time_filter::read_from_node(node);
	element_with_retry::read_from_node(node);
	ReadStrAttrIfExists(name, node, "name");
	ReadIntAttrIfExists(wait, node, "wait");
	ReadStrAttrIfExists(filename, node, "filename");
	ReadStrAttrIfExists(macroname, node, "macro");

	String rtype ;
	ReadStrAttrIfExists(rtype , node, "type");
	type = run_type_by_string(rtype);
	if (type==amp::rtUndefined) {
		throw Exception( "Unsupported run time occured: " + rtype+ ". run: " + name);
	}
}

run_item::run_item(): parent_model(NULL),send_run_params(NULL)
{
}

run_item::~run_item()
{
	delete send_run_params;
}

run_item::run_item(const run_item& src):send_run_params(NULL)
{
	*this = src;
}

run_item& run_item::operator =(const run_item& src)
{
	static_cast<run_item_data&>(*this) = src;
	if (src.send_run_params && src.send_run_params->size()) {
		if (!send_run_params) {
			send_run_params = new std::vector<send_run_element>(*src.send_run_params);
		} else
			*send_run_params = *src.send_run_params;
	} else
		if (send_run_params) {
			delete send_run_params;
			send_run_params = NULL;
		}
	return *this;
}

void run_item::set_parent(model& m)
{
	parent_model = &m;
}

model&		run_item::parent()
{
	if (!parent_model) {
		throw Exception("Parent model was not set.");
	}
	else
		return *parent_model;
}

void run_item::add_send_run_element(send_run_element& e)
{
	if (!send_run_params) {
		send_run_params = new std::vector<send_run_element>;
	}
	send_run_params->push_back(e);
}

send_run_element& run_item::get_send_run_element(int i)
{
	if (!send_run_params)
		throw Exception("No send elements defined.");
	else {
		return (*send_run_params)[i];
	}
}

int run_item::send_run_element_count()
{
	if (!send_run_params)
		return 0;
	else
		return send_run_params->size();
}

model_data::model_data()
{
}

void model_data::read_from_node(const TXMLNode& node)
{
	element_with_timeout::read_from_node(node);
	element_with_time_filter::read_from_node(node);
	ReadStrAttrIfExists(name, node, "name");
}

model::model()
{
	parent_module = NULL;
}

model::model(const model& src)
{
	*this = src;
}

model& model::operator = (const model& src)
{
	static_cast<model_data&>(*this) = src;
	run_elements = src.run_elements;
	for (int i=0; i < run_count(); i++)
		get_run(i).set_parent(*this);

	parent_module = src.parent_module;
	return *this  ;
}

control_module  &model::parent()
{
	if (!parent_module) {
		throw Exception("Parent module was not set.");
	}
	else
		return *parent_module;
}

void model::set_parent(control_module& p)
{
	parent_module = &p;
}

int model::run_count()
{
	return run_elements.size();
}

run_item& model::get_run(int i)
{
	return 	run_elements[i];
}

void model::add_run(run_item& item)
{
	item.set_parent(*this);
	item.set_index(run_count());
	run_elements.push_back(item);
}

control_module_data::control_module_data():
	enabled(true),
	loop(1)
{
}

control_module::control_module()
{
}

control_module::control_module(const control_module& src)
{
	*this = src;
}

control_module& control_module::operator = (const control_module& src)
{
	static_cast<control_module_data&>(*this) = src;
	run_models = src.run_models;
	for (int i=0; i < model_count(); i++)
		get_model(i).set_parent(*this);
	return *this;

}

int control_module::model_count()
{
	return run_models.size();
}

model& control_module::get_model(int i)
{
	return run_models[i]   ;
}

void control_module::add_model(model& item)
{
	item.set_index(model_count());
	item.set_parent(*this);
	run_models.push_back(item);
}

String control_module::def_node_caption()
{
	String result = "File: ";
	result+= ExtractFileName(filename);
	return result;
}

void control_module::load_from_file(const String& f)
{
	TXMLDocument * xmlDoc = new TXMLDocument(NULL);
	xmlDoc->DOMVendor = GetDOMVendor("ADOM XML v4");

	const _di_IXMLDocument document = interface_cast<Xmlintf::IXMLDocument>
		(xmlDoc);
	document->LoadFromFile(f.c_str());

	// Find a specific node.
	const _di_IXMLNode nodeControlModule =
		document->ChildNodes->FindNode("controlmodule");

	if (!nodeControlModule) {
		throw Exception("XML parsing error - no root controlmodule element");
	}
	*this = control_module();
	filename = f;
	ReadStrAttrIfExists(rootfolder, nodeControlModule, "rootfolder");
	ReadAttrValueIfExists(loop, nodeControlModule, "loop");
	ReadStrAttrIfExists(loop_logic, nodeControlModule, "loop_logic");
	ReadAttrValueIfExists(enabled, nodeControlModule, "enabled");

	if (nodeControlModule->HasChildNodes) {
		Xml::Xmlintf::_di_IXMLNodeList nodeModels = nodeControlModule->GetChildNodes();
		for (int i=0; i < nodeModels->GetCount(); i++) {
			Xml::Xmlintf::_di_IXMLNode nodeModel = nodeModels->GetNode(i);
			if (nodeModel->NodeName != String("model")) {
				continue;
			}
			model modelRead;
			modelRead.read_from_node(nodeModel);

			//	read run items for the model
			Xml::Xmlintf::_di_IXMLNodeList nodeRuns = nodeModel->GetChildNodes();
			for (int k=0; k < nodeRuns->GetCount(); k++) {
				Xml::Xmlintf::_di_IXMLNode nodeRun = nodeRuns->GetNode(k);
				if (nodeRun->NodeName!=String("run"))
					continue;

				run_item runRead;
				runRead.read_from_node(nodeRun);

				if ((runRead.type == rtCopy) && nodeRun->ChildNodes->Count) {
					Xml::Xmlintf::_di_IXMLNodeList nodeSends = nodeRun->GetChildNodes();
					for (Xml::Xmlintf::_di_IXMLNode nodeSend = nodeSends->Get(0);
								nodeSend ; nodeSend = nodeSend->NextSibling()) {
						if (nodeSend->NodeName!=String("send"))
							continue;

						send_run_element se;
						se.read_from_node(nodeSend);
						ReadBoolAttrIfExists(se.timestamp, nodeSend, "timestamp");
						ReadStrAttrIfExists(se.source, nodeSend, "source");
						ReadStrAttrIfExists(se.target, nodeSend, "target");
						ReadStrAttrIfExists(se.ziplevel, nodeSend, "ziplevel");
						if (se.ziplevel>9)
							se.ziplevel=9;
						runRead.add_send_run_element(se);
					}
				}
				modelRead.add_run(runRead);
			}

			add_model(modelRead);
		}
	}
//	delete xmlDoc;
//	nodeControlModule.Release();
}



const char* cszRunTypes [6] = { "mdb", "shell", "XML", "compact", "excel", "copy" };

run_type run_type_by_string(const String& s)
{
	int i=0;
	for (; i < rtUndefined; i++) {
		if (s.CompareIC(cszRunTypes[i])==0) {
			break  ;
		}
	}
	return (run_type)i;
}

String run_string_by_type(run_type rt)
{
	if (rt >= rtUndefined) {
		throw Exception("Run type out of range "+IntToStr(rt));
	}
	return cszRunTypes[rt];
}


String run_desc_by_type(run_type rt)
{
	static const char* sDesc[amp::rtUndefined] = {"calling an MS Access macro",
												  "calling any external app",
												  "calling another xml" ,
												  "compacting a MS Access mdb" ,
												  "calling an MS Excel macro",
												  "copying one file or folder to another"};
	if (rt > rtUndefined) {
		throw Exception("Run type out of range "+IntToStr(rt));
	}
	return sDesc[rt];
}

}


