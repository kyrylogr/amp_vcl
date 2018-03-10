//---------------------------------------------------------------------------

#ifndef control_module_structH
#define control_module_structH
//---------------------------------------------------------------------------

#include <string>
#include <vector>
#include <System.hpp>
#include <Xml.XMLDoc.hpp>

/**	Classes that describe controlmodule and its' nested elements(run, model).
*/

namespace amp {

typedef Xml::Xmlintf::_di_IXMLNode TXMLNode;

enum run_type { rtMDB,     //  calling an MS Access macro
				rtShell,   //  calling any external ap and can pass parameters to that ap
				rtXML,     //  Calling another xml forcing the code to branch
				rtCompact, //  compacting a MS Access mdb
				rtExcel,   //  calling an Excel file and run a macro
				rtCopy ,   //  copiing one file or folder to another, can also zip the file/folder
				rtUndefined} ;

run_type run_type_by_string(const String& s);

String run_string_by_type(run_type rt);

String run_desc_by_type(run_type rt);

/**	description of an element that can have timeout (used as base class for both run item and model)
*/

class 		element_with_timeout
{
public:
	int			timeout;
	bool		kill_on_timeout;
//	int			extra_wait_before_kill;
	element_with_timeout();
	void read_from_node(const TXMLNode& node);
};


/**	description of an elemet that can be retried
*/

class 		element_with_retry
{
public:
	int 		retries;
	int 		lockwait;
	element_with_retry();
	void read_from_node(const TXMLNode& node);
};


class element_with_index {
	int 		ctl_index;
public:
	element_with_index();
	void 		set_index(int i);
	int			index();
};

class element_with_time_filter {
public:
	bool   enabled;
	String enabled_logic;
	String days_of_week;
	String start_time;
	String end_time;
	element_with_time_filter();
	bool   is_ok_time(const TDateTime &t);
	bool   is_ok_day(const TDateTime &t);
	bool   is_allowed_by_filter(const TDateTime &t, String& message);
	void   read_from_node(const TXMLNode& node);
};

//	<send source="c:\Ansergy\Models\Reference.mdb" target="c:\Ansergy\Gdrive\System\FCsys\" ziplevel="6" timestamp="1" retries="1" lockwait="60"/>

/**	Sub element (send item) of copy run item
*	there can be several sent subitems in copy run.
*/
class send_run_element: public element_with_retry {
public:
	String source, target;
	String ziplevel;
	bool		timestamp;
	send_run_element();
};

class run_item_data: public element_with_timeout,
				public element_with_index,
				public element_with_time_filter,
				public element_with_retry {
public:
	int			wait;
	String 		filename;
	String 		macroname;
	run_type	type;
	String 		name;
	run_item_data();
	void read_from_node(const TXMLNode& node);
};

/**	Represents run item and its properties.
*/
class run_item: public run_item_data {
	friend class model;
	void set_parent(model& m);
	model*      parent_model;
	std::vector<send_run_element> *send_run_params;
public:
	run_item();
	run_item(const run_item& src);
	run_item& operator =(const run_item& src);
	virtual ~run_item();
	model&		parent();
	void add_send_run_element(send_run_element& e);
	send_run_element& get_send_run_element(int i);
	int  send_run_element_count();
};

/**	data of the model run (without run information)
*/

class model_data :  public element_with_timeout,
					public element_with_index,
					public element_with_time_filter {
public:
	String name;
	model_data();
	void read_from_node(const TXMLNode& node);
};

/**	Represents model part of configuration, its properties and nested run items.
*/
class model: public model_data {
	typedef std::vector<run_item> t_run_items;
	t_run_items run_elements;
	friend class control_module;
	control_module*      parent_module;
	void set_parent(control_module& p);
public:
	model();
	model(const model& src);
	model& operator = (const model& src);
	int run_count();
	run_item& get_run(int i);
	void add_run(run_item& item);
	control_module&		parent();

};

class control_module_data {
public:
	String filename;
	String loop_logic;
	String enabled_logic;
	String rootfolder;
	String days_of_week;
	bool		enabled;
	int			loop;
	control_module_data();
};

/**	Represents control_model, its properties and nested models.
*	has methods for loading from xml file and access to model items.
*/
class control_module: public control_module_data {
	typedef std::vector<model> t_models;
	t_models run_models;
	void add_model(model& item);

	control_module& operator = (const control_module& src);
	control_module(const control_module& src);
public:
	int model_count();
	model& get_model(int i);
	void load_from_file(const String& f);

	String def_node_caption();

	control_module();
};

}

#endif
