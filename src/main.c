// Error Lookup
// Copyright (c) 2011-2024 Henry++

#include "routine.h"

#include "main.h"
#include "rapp.h"

#include "resource.h"

STATIC_DATA config = {0};

VOID NTAPI _app_dereferencemoduleprocedure (
	_In_ PVOID entry
)
{
	PITEM_MODULE ptr_item;

	ptr_item = entry;

	if (ptr_item->file_name)
		_r_obj_dereference (ptr_item->file_name);

	if (ptr_item->full_path)
		_r_obj_dereference (ptr_item->full_path);

	if (ptr_item->description)
		_r_obj_dereference (ptr_item->description);

	if (ptr_item->string)
		_r_obj_dereference (ptr_item->string);

	if (ptr_item->hlib)
		_r_sys_freelibrary (ptr_item->hlib);
}

NTSTATUS _app_loadlibrary (
	_In_ PR_STRING name,
	_Out_ PVOID_PTR hmodule
)
{
	R_STORAGE storage;
	PVOID hlib;
	NTSTATUS status;

	status = _r_sys_loadlibraryasresource (name->buffer, &hlib);

	if (NT_SUCCESS (status))
	{
		status = _r_res_loadresource (hlib, RT_MESSAGETABLE, MAKEINTRESOURCEW (1), 0, &storage);

		if (!NT_SUCCESS (status))
		{
			_r_sys_freelibrary (hlib);

			hlib = NULL;
		}
	}

	*hmodule = hlib;

	return status;
}

PR_STRING _app_getfiledescription (
	_In_ PVOID hlib
)
{
	R_STORAGE ver_block;
	PR_STRING string;
	ULONG lcid;
	NTSTATUS status;

	status = _r_res_loadresource (hlib, RT_VERSION, MAKEINTRESOURCE (VS_VERSION_INFO), 0, &ver_block);

	if (!NT_SUCCESS (status))
		return _r_obj_referenceemptystring ();

	lcid = _r_res_querytranslation (ver_block.buffer);

	string = _r_res_querystring (ver_block.buffer, L"FileDescription", lcid);

	return string;
}

ULONG_PTR _app_getmodulehash (
	_In_ PR_STRING file_name,
	_Out_opt_ PR_STRING_PTR out_buffer
)
{
	PR_STRING path;
	ULONG_PTR module_hash = 0;
	NTSTATUS status;

	if (_r_fs_exists (file_name->buffer))
	{
		if (out_buffer)
			*out_buffer = _r_obj_reference (file_name);
	}
	else
	{
		status = _r_path_search (NULL, &file_name->sr, NULL, &path);

		if (NT_SUCCESS (status))
		{
			module_hash = _r_str_gethash2 (&path->sr, TRUE);

			if (out_buffer)
				*out_buffer = path;
		}
		else
		{
			if (out_buffer)
				*out_buffer = _r_obj_reference (file_name);
		}
	}

	if (!module_hash)
		module_hash = _r_str_gethash2 (&file_name->sr, TRUE);

	return module_hash;
}

VOID _app_addinfo (
	_Inout_ PR_XML_LIBRARY xml_library,
	_Inout_ PR_HASHTABLE hashtable
)
{
	PR_STRING text_value;
	LONG code;

	code = _r_xml_getattribute_long (xml_library, L"code");

	text_value = _r_xml_getattribute_string (xml_library, L"text");

	_r_obj_addhashtablepointer (hashtable, _r_math_hashinteger_ptr (code), text_value);
}

VOID _app_addmodule (
	_In_ HWND hwnd,
	_In_ PR_STRING file_name,
	_In_opt_ PR_STRING description,
	_In_opt_ PVOID hlib,
	_In_ BOOLEAN is_internal
)
{
	ITEM_MODULE mod = {0};
	PR_STRING path;
	ULONG_PTR module_hash;
	BOOLEAN is_enabled;
	NTSTATUS status;

	module_hash = _app_getmodulehash (file_name, &path);

	// module is already exists
	if (_r_obj_findhashtable (config.modules, module_hash))
	{
		_r_obj_dereference (path);

		return;
	}

	mod.file_name = file_name;
	mod.description = description;
	mod.full_path = path;
	mod.hlib = hlib;
	mod.is_internal = is_internal;

	is_enabled = _r_config_getboolean_ex (path->buffer, TRUE, SECTION_MODULE);

	if (is_enabled)
	{
		if (!mod.hlib)
		{
			status = _app_loadlibrary (path, &mod.hlib);

			if (NT_SUCCESS (status))
			{
				if (!mod.description)
					mod.description = _app_getfiledescription (mod.hlib);
			}
			else
			{
				if (status != STATUS_NO_SUCH_FILE && status != STATUS_MUI_FILE_NOT_FOUND && status != STATUS_RESOURCE_TYPE_NOT_FOUND)
					_r_show_errormessage (hwnd, NULL, status, path->buffer, ET_NATIVE);

				config.count_unload += 1;
			}
		}
	}
	else
	{
		config.count_unload += 1;
	}

	_r_obj_addhashtableitem (config.modules, module_hash, &mod);
}

BOOLEAN _app_checkmodule (
	_In_ PR_STRING file_name,
	_Out_ PULONG_PTR pmodule_hash
)
{
	ULONG_PTR module_hash;

	module_hash = _app_getmodulehash (file_name, NULL);

	*pmodule_hash = module_hash;

	if (_r_obj_findhashtable (config.modules, module_hash))
		return TRUE;

	return FALSE;
}

VOID _app_deletemodule (
	_In_ ULONG_PTR module_hash
)
{
	PITEM_MODULE ptr_module;

	ptr_module = _r_obj_findhashtable (config.modules, module_hash);

	if (!ptr_module)
		return;

	if (ptr_module->is_internal)
		return;

	_r_obj_removehashtableitem (config.modules, module_hash);
}

VOID _app_opendirectory (
	_In_ HWND hwnd,
	_In_ ULONG_PTR module_hash
)
{
	PITEM_MODULE ptr_module;
	NTSTATUS status;

	ptr_module = _r_obj_findhashtable (config.modules, module_hash);

	if (!ptr_module)
		return;

	if (!ptr_module->full_path)
	{
		if (ptr_module->file_name)
		{
			status = _r_path_search (NULL, &ptr_module->file_name->sr, NULL, &ptr_module->full_path);

			if (!NT_SUCCESS (status))
				_r_show_errormessage (hwnd, NULL, status, ptr_module->file_name->buffer, ET_NATIVE);
		}
	}

	if (ptr_module->full_path)
		_r_shell_showfile (ptr_module->full_path->buffer);
}

VOID _app_gettooltip (
	_Out_writes_ (buffer_length) LPWSTR buffer,
	_In_ ULONG_PTR buffer_length,
	_In_ LPARAM module_hash
)
{
	PITEM_MODULE ptr_module;

	ptr_module = _r_obj_findhashtable (config.modules, module_hash);

	if (!ptr_module)
	{
		*buffer = UNICODE_NULL;

		return;
	}

	if (!ptr_module->description)
		ptr_module->description = _app_getfiledescription (ptr_module->hlib);

	_r_str_printf (
		buffer,
		buffer_length,
		L"%s:\r\n    %s\r\n%s:\r\n    %s",
		_r_locale_getstring (IDS_FILE),
		_r_obj_getstringordefault (ptr_module->full_path, L"<empty>"),
		_r_locale_getstring (IDS_DESCRIPTION),
		_r_obj_getstringordefault (ptr_module->description, L"<empty>")
	);
}

INT CALLBACK _app_listviewcompare_callback (
	_In_ LPARAM lparam1,
	_In_ LPARAM lparam2,
	_In_ LPARAM lparam
)
{
	static R_STRINGREF sr1 = PR_STRINGREF_INIT (L"Windows (");

	WCHAR config_name[128];
	PR_STRING item_text_1;
	PR_STRING item_text_2;
	HWND hlistview;
	HWND hwnd;
	INT listview_id;
	INT column_id;
	INT result = 0;
	INT item1;
	INT item2;
	BOOLEAN is_descend;

	item1 = (INT)(INT_PTR)lparam1;
	item2 = (INT)(INT_PTR)lparam2;

	if (item1 == -1 || item2 == -1)
		return 0;

	hlistview = (HWND)lparam;

	hwnd = GetParent (hlistview);
	listview_id = GetDlgCtrlID (hlistview);

	_r_str_printf (config_name, RTL_NUMBER_OF (config_name), L"listview\\%04" TEXT (PRIX32), listview_id);

	column_id = _r_config_getlong_ex (L"SortColumn", 0, config_name);

	item_text_1 = _r_listview_getitemtext (hwnd, listview_id, item1, column_id);
	item_text_2 = _r_listview_getitemtext (hwnd, listview_id, item2, column_id);

	is_descend = _r_config_getboolean_ex (L"SortIsDescending", FALSE, config_name);

	if (item_text_1 && item_text_2)
	{
		if (listview_id == IDC_LISTVIEW)
		{
			if (_r_str_isstartswith (&item_text_1->sr, &sr1, TRUE))
			{
				result = -1;
			}
			else if (_r_str_isstartswith (&item_text_2->sr, &sr1, TRUE))
			{
				result = 1;
			}
		}

		if (!result)
			result = _r_str_compare_logical (item_text_1->buffer, item_text_2->buffer);
	}

	if (item_text_1)
		_r_obj_dereference (item_text_1);

	if (item_text_2)
		_r_obj_dereference (item_text_2);

	return is_descend ? -result : result;
}

VOID _app_listviewsort (
	_In_ HWND hwnd,
	_In_ INT listview_id,
	_In_ INT column_id,
	_In_ BOOLEAN is_notifycode
)
{
	WCHAR config_name[128];
	INT column_count;
	BOOLEAN is_descend;

	column_count = _r_listview_getcolumncount (hwnd, listview_id);

	if (!column_count)
		return;

	_r_str_printf (config_name, RTL_NUMBER_OF (config_name), L"listview\\%04" TEXT (PRIX32), listview_id);

	is_descend = _r_config_getboolean_ex (L"SortIsDescending", FALSE, config_name);

	if (is_notifycode)
		is_descend = !is_descend;

	if (column_id == -1)
		column_id = _r_config_getlong_ex (L"SortColumn", 0, config_name);

	column_id = _r_calc_clamp (column_id, 0, column_count - 1); // set range

	if (is_notifycode)
	{
		_r_config_setboolean_ex (L"SortIsDescending", is_descend, config_name);
		_r_config_setlong_ex (L"SortColumn", column_id, config_name);
	}

	for (INT i = 0; i < column_count; i++)
		_r_listview_setcolumnsortindex (hwnd, listview_id, i, 0);

	_r_listview_setcolumnsortindex (hwnd, listview_id, column_id, is_descend ? -1 : 1);

	_r_wnd_sendmessage (hwnd, listview_id, LVM_SORTITEMSEX, (WPARAM)GetDlgItem (hwnd, listview_id), (LPARAM)&_app_listviewcompare_callback);
}

VOID _app_refreshstatus (
	_In_ HWND hwnd
)
{
	ULONG_PTR modules_count;

	modules_count = _r_obj_gethashtablesize (config.modules);

	_r_status_settextformat (
		hwnd,
		IDC_STATUSBAR,
		0,
		_r_locale_getstring (IDS_STATUS_TOTAL),
		modules_count - config.count_unload,
		modules_count
	);
}

VOID _app_showdescription (
	_In_ HWND hwnd,
	_In_ ULONG_PTR module_hash
)
{
	PITEM_MODULE ptr_module;

	if (module_hash != SIZE_MAX)
	{
		ptr_module = _r_obj_findhashtable (config.modules, module_hash);

		if (ptr_module)
		{
			_r_ctrl_setstringformat (
				hwnd,
				IDC_DESCRIPTION_CTL,
				L"%s\r\n\r\n%s",
				config.info,
				_r_obj_getstringorempty (ptr_module->string)
			);

			_r_status_settextformat (
				hwnd,
				IDC_STATUSBAR,
				1,
				L"%s - %s",
				_r_obj_getstringorempty (ptr_module->description),
				_r_obj_getstringorempty (ptr_module->file_name)
			);
		}
	}
	else
	{
		_r_ctrl_setstring (hwnd, IDC_DESCRIPTION_CTL, config.info);

		_r_status_settext (hwnd, IDC_STATUSBAR, 1, NULL);
	}
}

VOID _app_print (
	_In_ HWND hwnd
)
{
	PITEM_MODULE ptr_module = NULL;
	PR_STRING severity_string;
	PR_STRING facility_string;
	PR_STRING string;
	ULONG_PTR enum_key = 0;
	ULONG_PTR module_hash;
	LONG severity_code;
	LONG facility_code;
	LONG error_code;
	INT item_count = 0;
	INT select_id;
	NTSTATUS status;

	error_code = _r_ctrl_getinteger (hwnd, IDC_CODE_CTL, NULL);

	severity_code = HRESULT_SEVERITY (error_code);
	facility_code = HRESULT_FACILITY (error_code);

	severity_string = _r_obj_findhashtablepointer (config.severity, _r_math_hashinteger_ptr (severity_code));
	facility_string = _r_obj_findhashtablepointer (config.facility, _r_math_hashinteger_ptr (facility_code));

	select_id = _r_config_getlong (L"SelectedItem", -1);

	if (select_id == -1)
		select_id = _r_listview_getselecteditem (hwnd, IDC_LISTVIEW);

	// clear first
	_r_listview_deleteallitems (hwnd, IDC_LISTVIEW);

	// print information
	_r_str_printf (
		config.info, RTL_NUMBER_OF (config.info),
		L"Code (dec.): " FORMAT_DEC L"\r\nCode (hex.): " FORMAT_HEX L"\r\n" \
		L"\r\nSeverity:\r\n%s (0x%02" TEXT (PRIX32) L")\r\n\r\nFacility:\r\n%s (0x%02" TEXT (PRIX32) L")",
		error_code,
		error_code,
		_r_obj_getstringordefault (severity_string, L"n/a"),
		severity_code,
		_r_obj_getstringordefault (facility_string, L"n/a"),
		facility_code
	);

	_r_ctrl_setstring (hwnd, IDC_DESCRIPTION_CTL, config.info);

	// print modules
	while (_r_obj_enumhashtable (config.modules, &ptr_module, &module_hash, &enum_key))
	{
		if (!ptr_module->hlib || !ptr_module->full_path || !_r_config_getboolean_ex (ptr_module->full_path->buffer, TRUE, SECTION_MODULE))
			continue;

		status = _r_sys_formatmessage (error_code, ptr_module->hlib, config.lcid, &string);

		_r_obj_movereference (&ptr_module->string, string);

		if (NT_SUCCESS (status))
		{
			_r_listview_additem_ex (
				hwnd,
				IDC_LISTVIEW,
				item_count,
				_r_obj_getstringordefault (ptr_module->description, ptr_module->file_name->buffer),
				I_IMAGENONE,
				ptr_module->is_internal ? 0 : 1,
				module_hash
			);

			item_count += 1;
		}
	}

	if (select_id == -1)
		select_id = 0;

	_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, NULL, -100);

	_app_listviewsort (hwnd, IDC_LISTVIEW, -1, FALSE);

	// show description for first item
	if (item_count)
	{
		_r_listview_setitemstate (hwnd, IDC_LISTVIEW, select_id, LVIS_SELECTED | LVIS_ACTIVATING, LVIS_SELECTED | LVIS_ACTIVATING); // select item
	}
	else
	{
		_app_showdescription (hwnd, SIZE_MAX);
	}

	if (severity_string)
		_r_obj_dereference (severity_string);

	if (facility_string)
		_r_obj_dereference (facility_string);
}

VOID _app_loaddatabase (
	_In_ HWND hwnd
)
{
	static R_STRINGREF whitespace = PR_STRINGREF_INIT (L"\r\n ");

	R_XML_LIBRARY xml_library;
	WCHAR buffer[512];
	R_STRINGREF remaining_part;
	R_STRINGREF value_part;
	PR_STRING file_value;
	PR_STRING text_value;
	PR_STRING string;
	R_STORAGE bytes;
	PR_STRING path;
	HRESULT status;

	config.count_unload = 0;

	if (!config.modules)
	{
		config.modules = _r_obj_createhashtable_ex (sizeof (ITEM_MODULE), 128, &_app_dereferencemoduleprocedure);
	}
	else
	{
		_r_obj_clearhashtable (config.modules);
	}

	if (!config.facility)
	{
		config.facility = _r_obj_createhashtablepointer (64);
	}
	else
	{
		_r_obj_clearhashtable (config.facility);
	}

	if (!config.severity)
	{
		config.severity = _r_obj_createhashtablepointer (64);
	}
	else
	{
		_r_obj_clearhashtable (config.severity);
	}

	// load internal modules
	status = _r_xml_initializelibrary (&xml_library, TRUE);

	if (SUCCEEDED (status))
	{
		_r_str_printf (buffer, RTL_NUMBER_OF (buffer), L"%s\\modules.xml", _r_app_getdirectory ()->buffer);

		if (_r_fs_exists (buffer))
		{
			status = _r_xml_parsefile (&xml_library, buffer);
		}
		else
		{
			status = COMADMIN_E_COMPFILE_DOESNOTEXIST;
		}

		if (FAILED (status))
		{
			status = _r_res_loadresource (_r_sys_getimagebase (), RT_RCDATA, MAKEINTRESOURCEW (1), 0, &bytes);

			if (NT_SUCCESS (status))
			{
				// TODO: unpack!

				status = _r_xml_parsestring (&xml_library, bytes.buffer, bytes.length);
			}
			else
			{
				_r_show_errormessage (hwnd, NULL, status, NULL, ET_NATIVE);
			}
		}

		if (SUCCEEDED (status))
		{
			_r_xml_findchildbytagname (&xml_library, L"root");

			if (_r_xml_findchildbytagname (&xml_library, L"module"))
			{
				while (_r_xml_enumchilditemsbytagname (&xml_library, L"item"))
				{
					file_value = _r_xml_getattribute_string (&xml_library, L"file");

					if (!file_value)
						continue;

					text_value = _r_xml_getattribute_string (&xml_library, L"text");

					_app_addmodule (hwnd, file_value, text_value, NULL, TRUE);
				}
			}

			if (_r_xml_findchildbytagname (&xml_library, L"facility"))
			{
				while (_r_xml_enumchilditemsbytagname (&xml_library, L"item"))
				{
					_app_addinfo (&xml_library, config.facility);
				}
			}

			if (_r_xml_findchildbytagname (&xml_library, L"severity"))
			{
				while (_r_xml_enumchilditemsbytagname (&xml_library, L"item"))
				{
					_app_addinfo (&xml_library, config.severity);
				}
			}
		}

		_r_xml_destroylibrary (&xml_library);
	}
	else
	{
		_r_show_errormessage (hwnd, NULL, status, NULL, ET_WINDOWS);
	}

	// load external modules
	string = _r_config_getstring (L"Modules", NULL);

	if (string)
	{
		_r_obj_initializestringref2 (&remaining_part, &string->sr);

		while (remaining_part.length != 0)
		{
			_r_str_splitatchar (&remaining_part, L';', &value_part, &remaining_part);

			_r_str_trimstring (&value_part, &whitespace, 0);

			if (value_part.length > 4 * sizeof (WCHAR))
			{
				path = _r_obj_createstring2 (&value_part);

				if (path)
					_app_addmodule (hwnd, path, NULL, NULL, FALSE);
			}
		}

		_r_obj_dereference (string);
	}

	_app_refreshstatus (hwnd);
}

VOID _app_additemtolist (
	_In_ HWND hwnd,
	_In_ PR_STRING path,
	_In_ ULONG_PTR module_hash,
	_In_ BOOLEAN is_internal
)
{
	INT index;

	index = _r_listview_getitemcount (hwnd, IDC_MODULES);

	_r_listview_additem_ex (hwnd, IDC_MODULES, index, _r_path_getbasename (&path->sr), I_IMAGENONE, is_internal ? 0 : 1, module_hash);

	if (_r_config_getboolean_ex (path->buffer, TRUE, SECTION_MODULE))
		_r_listview_setitemcheck (hwnd, IDC_MODULES, index, TRUE);
}

INT_PTR CALLBACK SettingsProc (
	_In_ HWND hwnd,
	_In_ UINT msg,
	_In_ WPARAM wparam,
	_In_ LPARAM lparam
)
{
	switch (msg)
	{
		case RM_INITIALIZE:
		{
			switch ((INT)wparam)
			{
				case IDD_MODULES:
				{
					PITEM_MODULE ptr_module = NULL;
					ULONG_PTR enum_key = 0;
					ULONG_PTR module_hash;
					ULONG style = LVS_EX_CHECKBOXES | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP;

					_r_listview_deleteallitems (hwnd, IDC_MODULES);
					_r_listview_deleteallcolumns (hwnd, IDC_MODULES);

					_r_listview_setstyle (hwnd, IDC_MODULES, style, TRUE);

					_r_listview_addcolumn (hwnd, IDC_MODULES, 0, L"", -100, LVCFMT_LEFT);

					_r_listview_addgroup (hwnd, IDC_MODULES, 0, _r_locale_getstring (IDS_MODULES_INT), 0, LVGS_COLLAPSIBLE, LVGS_COLLAPSIBLE);
					_r_listview_addgroup (hwnd, IDC_MODULES, 1, _r_locale_getstring (IDS_MODULES_EXT), 0, LVGS_COLLAPSIBLE, LVGS_COLLAPSIBLE);

					while (_r_obj_enumhashtable (config.modules, &ptr_module, &module_hash, &enum_key))
					{
						_app_additemtolist (hwnd, ptr_module->full_path, module_hash, ptr_module->is_internal);
					}

					_app_listviewsort (hwnd, IDC_MODULES, -1, FALSE);

					break;
				}
			}

			break;
		}

		case RM_LOCALIZE:
		{
			switch ((INT)wparam)
			{
				case IDD_MODULES:
				{
					_r_listview_setcolumn (hwnd, IDC_MODULES, 0, _r_locale_getstring (IDS_FILE), -100);
					break;
				}
			}

			break;
		}

		case RM_CONFIG_SAVE:
		{

			switch ((INT)wparam)
			{
				case IDD_MODULES:
				{
					R_STRINGBUILDER sb;
					PITEM_MODULE ptr_module;
					PR_STRING string;
					HWND hmain;
					ULONG_PTR module_hash;
					INT item_count;
					INT group_id;
					BOOLEAN is_enabled;
					NTSTATUS status;

					item_count = _r_listview_getitemcount (hwnd, IDC_MODULES);

					if (!item_count)
						break;

					for (INT i = 0; i < item_count; i++)
					{
						module_hash = _r_listview_getitemlparam (hwnd, IDC_MODULES, i);
						ptr_module = _r_obj_findhashtable (config.modules, module_hash);

						if (!ptr_module)
							continue;

						is_enabled = _r_listview_isitemchecked (hwnd, IDC_MODULES, i);

						if (is_enabled == _r_config_getboolean_ex (ptr_module->full_path->buffer, TRUE, SECTION_MODULE))
							continue;

						_r_config_setboolean_ex (ptr_module->full_path->buffer, is_enabled, SECTION_MODULE);

						if (is_enabled)
						{
							if (!ptr_module->hlib)
							{
								status = _app_loadlibrary (ptr_module->full_path, &ptr_module->hlib);

								if (NT_SUCCESS (status))
								{
									config.count_unload -= 1;
								}
								else
								{
									_r_show_errormessage (hwnd, NULL, status, ptr_module->full_path->buffer, ET_NATIVE);
								}
							}
						}
						else
						{
							SAFE_DELETE_LIBRARY (ptr_module->hlib);
							SAFE_DELETE_REFERENCE (ptr_module->string);

							config.count_unload += 1;
						}
					}

					_r_obj_initializestringbuilder (&sb, 256);

					for (INT i = 0; i < item_count; i++)
					{
						group_id = _r_listview_getitemgroup (hwnd, IDC_MODULES, i);

						if (group_id == 0)
							continue;

						string = _r_listview_getitemtext (hwnd, IDC_MODULES, i, 0);

						if (!string)
							continue;

						_r_obj_appendstringbuilder2 (&sb, &string->sr);
						_r_obj_appendstringbuilder (&sb, L";");

						_r_obj_dereference (string);
					}

					string = _r_obj_finalstringbuilder (&sb);

					_r_config_setstring (L"Modules", _r_obj_getstring (string));

					_r_obj_deletestringbuilder (&sb);

					_app_loaddatabase (hwnd);

					hmain = _r_app_gethwnd ();

					if (!hmain)
						break;

					_app_refreshstatus (hmain);

					_app_print (hmain);

					break;
				}
			}

			break;
		}

		case WM_CONTEXTMENU:
		{
			LPNMITEMACTIVATE lpnmlv;
			HMENU hsubmenu;
			HMENU hmenu;
			INT group_id;
			INT item_id;

			lpnmlv = (LPNMITEMACTIVATE)lparam;

			if (GetDlgCtrlID ((HWND)wparam) != IDC_MODULES)
				break;

			// localize
			hmenu = LoadMenuW (NULL, MAKEINTRESOURCEW (IDM_LISTVIEW));

			if (!hmenu)
				break;

			hsubmenu = GetSubMenu (hmenu, 0);

			if (hsubmenu)
			{
				item_id = _r_listview_getselecteditem (hwnd, IDC_MODULES);

				group_id = _r_listview_getitemgroup (hwnd, IDC_MODULES, item_id);

				_r_menu_setitemtextformat (hsubmenu, IDM_ADD, FALSE, L"%s...", _r_locale_getstring (IDS_ADD));
				_r_menu_setitemtext (hsubmenu, IDM_DELETE, FALSE, _r_locale_getstring (IDS_DELETE));

				_r_menu_setitemtext (hsubmenu, IDM_CHECK, FALSE, _r_locale_getstring (IDS_CHECK));
				_r_menu_setitemtext (hsubmenu, IDM_UNCHECK, FALSE, _r_locale_getstring (IDS_UNCHECK));

				_r_menu_setitemtext (hsubmenu, IDM_EXPLORE, FALSE, _r_locale_getstring (IDS_EXPLORE));
				_r_menu_setitemtext (hsubmenu, IDM_COPY, FALSE, _r_locale_getstring (IDS_COPY));

				if (group_id == 0)
					_r_menu_enableitem (hsubmenu, IDM_DELETE, MF_BYCOMMAND, FALSE);

				if (!_r_listview_getselectedcount (hwnd, IDC_MODULES))
				{
					_r_menu_enableitem (hsubmenu, IDM_DELETE, MF_BYCOMMAND, FALSE);

					_r_menu_enableitem (hsubmenu, IDM_CHECK, MF_BYCOMMAND, FALSE);
					_r_menu_enableitem (hsubmenu, IDM_UNCHECK, MF_BYCOMMAND, FALSE);

					_r_menu_enableitem (hsubmenu, IDM_EXPLORE, MF_BYCOMMAND, FALSE);
					_r_menu_enableitem (hsubmenu, IDM_COPY, MF_BYCOMMAND, FALSE);
				}

				_r_menu_popup (hsubmenu, hwnd, NULL, TRUE);
			}

			DestroyMenu (hmenu);

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR lphdr = (LPNMHDR)lparam;

			switch (lphdr->code)
			{
				case NM_DBLCLK:
				{
					LPNMITEMACTIVATE lpnmlv;

					if (lphdr->idFrom != IDC_MODULES)
						break;

					lpnmlv = (LPNMITEMACTIVATE)lparam;

					if (lpnmlv->iItem == -1)
						break;

					_r_wnd_sendmessage (hwnd, 0, WM_COMMAND, MAKEWPARAM (IDM_EXPLORE, 0), 0);

					break;
				}

				case NM_CUSTOMDRAW:
				{
					LPNMLVCUSTOMDRAW lpnmlv;
					LONG_PTR result = CDRF_DODEFAULT;

					if (lphdr->idFrom != IDC_MODULES)
						break;

					lpnmlv = (LPNMLVCUSTOMDRAW)lparam;

					switch (lpnmlv->nmcd.dwDrawStage)
					{
						case CDDS_PREPAINT:
						{
							result = CDRF_NOTIFYITEMDRAW;
							break;
						}

						case CDDS_ITEMPREPAINT:
						{
							PITEM_MODULE ptr_module;
							ULONG new_clr;

							if (lpnmlv->dwItemType != LVCDI_ITEM)
								break;

							ptr_module = _r_obj_findhashtable (config.modules, lpnmlv->nmcd.lItemlParam);

							if (!ptr_module)
								break;

							if (_r_listview_isitemchecked (hwnd, IDC_MODULES, (INT)lpnmlv->nmcd.dwItemSpec))
							{
								if (!ptr_module->hlib)
								{
									new_clr = WND_GRAYTEXT_CLR;

									lpnmlv->clrTextBk = new_clr;
									lpnmlv->clrText = _r_dc_getcolorbrightness (new_clr);

									result = CDRF_NEWFONT;
								}
							}

							break;
						}

						break;
					}

					SetWindowLongPtrW (hwnd, DWLP_MSGRESULT, result);

					return result;
				}

				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIPW lpnmlv;
					LPARAM module_hash;

					lpnmlv = (LPNMLVGETINFOTIPW)lparam;

					if (lpnmlv->hdr.idFrom != IDC_MODULES)
						break;

					module_hash = _r_listview_getitemlparam (hwnd, IDC_MODULES, lpnmlv->iItem);

					_app_gettooltip (lpnmlv->pszText, lpnmlv->cchTextMax, module_hash);

					break;
				}

				case LVN_COLUMNCLICK:
				{
					LPNMLISTVIEW lpnmlv;
					INT ctrl_id;

					lpnmlv = (LPNMLISTVIEW)lparam;
					ctrl_id = (INT)(INT_PTR)lpnmlv->hdr.idFrom;

					if (ctrl_id != IDC_MODULES)
						break;

					_app_listviewsort (hwnd, ctrl_id, lpnmlv->iSubItem, TRUE);

					break;
				}
			}

			break;
		}

		case WM_COMMAND:
		{
			switch (LOWORD (wparam))
			{
				case IDM_ADD:
				{
					static COMDLG_FILTERSPEC filters[] = {
						L"Dll files (*.dll)", L"*.dll",
						L"All files (*.*)", L"*.*",
					};

					R_FILE_DIALOG file_dialog;
					PR_STRING path;
					PVOID hlib;
					ULONG_PTR module_hash;
					NTSTATUS status;

					status = _r_filedialog_initialize (&file_dialog, PR_FILEDIALOG_OPENFILE);

					if (SUCCEEDED (status))
					{
						_r_filedialog_setfilter (&file_dialog, filters, RTL_NUMBER_OF (filters));

						status = _r_filedialog_show (hwnd, &file_dialog);

						if (SUCCEEDED (status))
						{
							status = _r_filedialog_getpath (&file_dialog, &path);

							if (SUCCEEDED (status))
							{
								if (_app_checkmodule (path, &module_hash))
								{
									_r_show_errormessage (hwnd, NULL, STATUS_OBJECT_NAME_COLLISION, path->buffer, ET_NATIVE);
								}
								else
								{
									status = _app_loadlibrary (path, &hlib);

									if (NT_SUCCESS (status))
									{
										_app_addmodule (hwnd, path, NULL, hlib, FALSE);

										_app_additemtolist (hwnd, path, module_hash, FALSE);
									}
									else
									{
										_r_show_errormessage (hwnd, NULL, status, path->buffer, ET_NATIVE);
									}
								}

								_r_obj_dereference (path);
							}
						}

						_r_filedialog_destroy (&file_dialog);
					}

					break;
				}

				case IDM_DELETE:
				{
					INT item_count;

					if (!_r_listview_getselectedcount (hwnd, IDC_MODULES))
						break;

					if (!_r_show_confirmmessage (hwnd, NULL, _r_locale_getstring (IDS_QUESTION_DELETE), NULL))
						break;

					item_count = _r_listview_getitemcount (hwnd, IDC_MODULES);

					for (INT i = item_count - 1; i != -1; i--)
					{
						if (!_r_listview_isitemselected (hwnd, IDC_MODULES, i))
							continue;

						if (_r_listview_getitemgroup (hwnd, IDC_MODULES, i) != 1)
							continue;

						_r_listview_deleteitem (hwnd, IDC_MODULES, i);
					}

					break;
				}

				case IDM_CHECK:
				case IDM_UNCHECK:
				{
					INT item_id = -1;

					if (!_r_listview_getselectedcount (hwnd, IDC_MODULES))
						break;

					while ((item_id = _r_listview_getnextselected (hwnd, IDC_MODULES, item_id)) != -1)
					{
						_r_listview_setitemcheck (hwnd, IDC_MODULES, item_id, LOWORD (wparam) == IDM_CHECK ? TRUE : FALSE);
					}

					break;
				}

				case IDM_EXPLORE:
				{
					ULONG_PTR module_hash;
					INT item_id = -1;

					if (!_r_listview_getselectedcount (hwnd, IDC_MODULES))
						break;

					while ((item_id = _r_listview_getnextselected (hwnd, IDC_MODULES, item_id)) != -1)
					{
						module_hash = _r_listview_getitemlparam (hwnd, IDC_MODULES, item_id);

						_app_opendirectory (hwnd, module_hash);
					}

					break;
				}

				case IDM_COPY:
				{
					R_STRINGBUILDER sb;
					PR_STRING string;
					INT item_id = -1;

					if (!_r_listview_getselectedcount (hwnd, IDC_MODULES))
						break;

					_r_obj_initializestringbuilder (&sb, 256);

					while ((item_id = _r_listview_getnextselected (hwnd, IDC_MODULES, item_id)) != -1)
					{
						string = _r_listview_getitemtext (hwnd, IDC_MODULES, item_id, 0);

						if (string)
						{
							_r_obj_appendstringbuilder2 (&sb, &string->sr);
							_r_obj_appendstringbuilder (&sb, L"\r\n");

							_r_obj_dereference (string);
						}
					}

					string = _r_obj_finalstringbuilder (&sb);

					_r_str_trimstring2 (&string->sr, L"\r\n", 0);

					_r_clipboard_set (hwnd, &string->sr);

					_r_obj_deletestringbuilder (&sb);

					break;
				}
			}

			break;
		}
	}

	return FALSE;
}

INT_PTR CALLBACK DlgProc (
	_In_ HWND hwnd,
	_In_ UINT msg,
	_In_ WPARAM wparam,
	_In_ LPARAM lparam
)
{
	static R_LAYOUT_MANAGER layout_manager = {0};

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			PR_STRING string;
			ULONG style = LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP;

			_r_app_sethwnd (hwnd); // HACK!!!

			// configure listview
			_r_listview_setstyle (hwnd, IDC_LISTVIEW, style, TRUE);

			_r_listview_addcolumn (hwnd, IDC_LISTVIEW, 0, _r_locale_getstring (IDS_MODULES), 100, LVCFMT_LEFT);

			_r_listview_addgroup (hwnd, IDC_LISTVIEW, 0, _r_locale_getstring (IDS_MODULES_INT), 0, LVGS_COLLAPSIBLE, LVGS_COLLAPSIBLE);
			_r_listview_addgroup (hwnd, IDC_LISTVIEW, 1, _r_locale_getstring (IDS_MODULES_EXT), 0, LVGS_COLLAPSIBLE, LVGS_COLLAPSIBLE);

			// configure controls
			_r_updown_setrange (hwnd, IDC_CODE_UD, 0, LONG_MAX);

			// set error code text
			if (_r_config_getboolean (L"InsertBufferAtStartup", FALSE))
			{
				string = _r_clipboard_get (hwnd);

				if (string)
				{
					_r_str_trimstring2 (&string->sr, L"\r\n ", 0);

					if (_r_obj_isstringempty2 (string))
						_r_obj_clearreference (&string);
				}
			}
			else
			{
				string = NULL;
			}

			if (!string)
				string = _r_config_getstring (L"LatestCode", L"0x00000000");

			if (string)
			{
				_r_ctrl_setstring (hwnd, IDC_CODE_CTL, string->buffer);

				_r_obj_dereference (string);
			}

			// configure settings
			_r_settings_addpage (IDD_MODULES, IDS_MODULES);

			// initialize layout manager
			_r_layout_initializemanager (&layout_manager, hwnd);

			break;
		}

		case RM_INITIALIZE:
		{
			HMENU hmenu;

			// load xml database
			_app_loaddatabase (hwnd);

			// configure menu
			hmenu = GetMenu (hwnd);

			if (!hmenu)
				break;

			_r_menu_checkitem (hmenu, IDM_ALWAYSONTOP_CHK, 0, MF_BYCOMMAND, _r_config_getboolean (L"AlwaysOnTop", FALSE));
			_r_menu_checkitem (hmenu, IDM_DARKMODE_CHK, 0, MF_BYCOMMAND, _r_theme_isenabled ());
			_r_menu_checkitem (hmenu, IDM_INSERTBUFFER_CHK, 0, MF_BYCOMMAND, _r_config_getboolean (L"InsertBufferAtStartup", FALSE));
			_r_menu_checkitem (hmenu, IDM_CHECKUPDATES_CHK, 0, MF_BYCOMMAND, _r_update_isenabled (FALSE));

			break;
		}

		case RM_LOCALIZE:
		{
			// get locale id
			PR_STRING string;
			HMENU hmenu;
			LONG64 value;
			ULONG number;

			string = _r_locale_getstring_ex (IDS_LCID);

			if (string)
			{
				if (_r_str_tointeger64 (&string->sr, 0, NULL, &value))
				{
					if (SUCCEEDED (LongLongToULong (value, &number)))
						config.lcid = number;
				}

				_r_obj_dereference (string);
			}
			else
			{
				config.lcid = 0;
			}

			_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, _r_locale_getstring (IDS_MODULES), 0);

			// localize
			hmenu = GetMenu (hwnd);

			if (hmenu)
			{
				_r_menu_setitemtext (hmenu, 0, TRUE, _r_locale_getstring (IDS_FILE));
				_r_menu_setitemtext (hmenu, 1, TRUE, _r_locale_getstring (IDS_SETTINGS));
				_r_menu_setitemtext (hmenu, 2, TRUE, _r_locale_getstring (IDS_HELP));

				_r_menu_setitemtextformat (hmenu, IDM_SETTINGS, FALSE, L"%s...\tF2", _r_locale_getstring (IDS_SETTINGS));
				_r_menu_setitemtextformat (hmenu, IDM_EXIT, FALSE, L"%s\tEsc", _r_locale_getstring (IDS_EXIT));
				_r_menu_setitemtext (hmenu, IDM_ALWAYSONTOP_CHK, FALSE, _r_locale_getstring (IDS_ALWAYSONTOP_CHK));
				_r_menu_setitemtext (hmenu, IDM_DARKMODE_CHK, FALSE, _r_locale_getstring (IDS_DARKMODE_CHK));
				_r_menu_setitemtext (hmenu, IDM_INSERTBUFFER_CHK, FALSE, _r_locale_getstring (IDS_INSERTBUFFER_CHK));
				_r_menu_setitemtext (hmenu, IDM_CHECKUPDATES_CHK, FALSE, _r_locale_getstring (IDS_CHECKUPDATES_CHK));
				_r_menu_setitemtextformat (GetSubMenu (hmenu, 1), LANG_MENU, TRUE, L"%s (Language)", _r_locale_getstring (IDS_LANGUAGE));
				_r_menu_setitemtext (hmenu, IDM_WEBSITE, FALSE, _r_locale_getstring (IDS_WEBSITE));
				_r_menu_setitemtext (hmenu, IDM_CHECKUPDATES, FALSE, _r_locale_getstring (IDS_CHECKUPDATES));
				_r_menu_setitemtextformat (hmenu, IDM_ABOUT, FALSE, L"%s\tF1", _r_locale_getstring (IDS_ABOUT));

				_r_locale_enum (GetSubMenu (hmenu, 1), LANG_MENU, IDX_LANGUAGE); // enum localizations
			}

			_r_ctrl_setstringformat (hwnd, IDC_CODE, L"%s:", _r_locale_getstring (IDS_CODE));
			_r_ctrl_setstringformat (hwnd, IDC_DESCRIPTION, L"%s:", _r_locale_getstring (IDS_DESCRIPTION));

			_app_print (hwnd);

			_app_refreshstatus (hwnd);

			break;
		}

		case WM_DESTROY:
		{
			PR_STRING window_text;

			window_text = _r_ctrl_getstring (hwnd, IDC_CODE_CTL);

			_r_config_setlong (L"SelectedItem", _r_listview_getselecteditem (hwnd, IDC_LISTVIEW));

			_r_config_setstring (L"LatestCode", _r_obj_getstring (window_text));

			if (window_text)
				_r_obj_dereference (window_text);

			PostQuitMessage (0);

			break;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;

			hdc = BeginPaint (hwnd, &ps);

			if (!hdc)
				break;

			_r_dc_drawwindow (hdc, hwnd, FALSE);

			EndPaint (hwnd, &ps);

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR nmlp;

			nmlp = (LPNMHDR)lparam;

			switch (nmlp->code)
			{
				case NM_RCLICK:
				{
					LPNMITEMACTIVATE lpnmlv;
					HMENU hsubmenu;
					HMENU hmenu;

					lpnmlv = (LPNMITEMACTIVATE)lparam;

					if (!nmlp->idFrom || lpnmlv->iItem == -1 || nmlp->idFrom != IDC_LISTVIEW)
						break;

					// localize
					hmenu = LoadMenuW (NULL, MAKEINTRESOURCEW (IDM_LISTVIEW));

					if (!hmenu)
						break;

					hsubmenu = GetSubMenu (hmenu, 0);

					if (hsubmenu)
					{
						_r_menu_deleteitem (hsubmenu, IDM_CHECK, FALSE);
						_r_menu_deleteitem (hsubmenu, IDM_UNCHECK, FALSE);
						_r_menu_deleteitem (hsubmenu, 0, TRUE);
						_r_menu_deleteitem (hsubmenu, IDM_ADD, FALSE);
						_r_menu_deleteitem (hsubmenu, IDM_DELETE, FALSE);
						_r_menu_deleteitem (hsubmenu, 0, TRUE);

						_r_menu_setitemtextformat (hsubmenu, IDM_EXPLORE, FALSE, L"%s\tCtrl+E", _r_locale_getstring (IDS_EXPLORE));
						_r_menu_setitemtextformat (hsubmenu, IDM_COPY, FALSE, L"%s\tCtrl+C", _r_locale_getstring (IDS_COPY));

						_r_menu_popup (hsubmenu, hwnd, NULL, TRUE);
					}

					DestroyMenu (hmenu);

					break;
				}

				case NM_DBLCLK:
				{
					LPNMITEMACTIVATE lpnmlv;

					if (nmlp->idFrom != IDC_LISTVIEW)
						break;

					lpnmlv = (LPNMITEMACTIVATE)lparam;

					if (lpnmlv->iItem == -1)
						break;

					_r_wnd_sendmessage (hwnd, 0, WM_COMMAND, MAKEWPARAM (IDM_EXPLORE, 0), 0);

					break;
				}

				case NM_CLICK:
				case LVN_ITEMCHANGED:
				{
					LPNMITEMACTIVATE lpnmlv;
					ULONG_PTR module_hash;

					if (nmlp->idFrom != IDC_LISTVIEW)
						break;

					lpnmlv = (LPNMITEMACTIVATE)lparam;

					if (lpnmlv->iItem != -1)
					{
						module_hash = _r_listview_getitemlparam (hwnd, IDC_LISTVIEW, lpnmlv->iItem);

						_app_showdescription (hwnd, module_hash);
					}
					else
					{
						_app_showdescription (hwnd, SIZE_MAX);
					}

					break;
				}

				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIPW lpnmlv;
					LPARAM module_hash;

					if (nmlp->idFrom != IDC_LISTVIEW)
						break;

					lpnmlv = (LPNMLVGETINFOTIPW)lparam;

					module_hash = _r_listview_getitemlparam (hwnd, IDC_LISTVIEW, lpnmlv->iItem);

					_app_gettooltip (lpnmlv->pszText, lpnmlv->cchTextMax, module_hash);

					break;
				}

				case LVN_COLUMNCLICK:
				{
					LPNMLISTVIEW lpnmlv;
					INT ctrl_id;

					if (nmlp->idFrom != IDC_LISTVIEW)
						break;

					lpnmlv = (LPNMLISTVIEW)lparam;
					ctrl_id = (INT)(INT_PTR)nmlp->idFrom;

					_app_listviewsort (hwnd, ctrl_id, lpnmlv->iSubItem, TRUE);

					break;
				}

				case UDN_DELTAPOS:
				{
					LPNMUPDOWN lpnmud;
					ULONG base;
					LONG code;

					if (nmlp->idFrom != IDC_CODE_UD)
						break;

					lpnmud = (LPNMUPDOWN)lparam;

					code = _r_ctrl_getinteger (hwnd, IDC_CODE_CTL, &base);

					_r_ctrl_setstringformat (hwnd, IDC_CODE_CTL, base != 10 ? FORMAT_HEX : FORMAT_DEC, code + lpnmud->iDelta);

					_app_print (hwnd);

					return TRUE;
				}
			}

			break;
		}

		case WM_SIZE:
		{
			LONG width;

			if (!_r_layout_resize (&layout_manager, wparam))
				break;

			// resize statusbar parts
			width = _r_ctrl_getwidth (hwnd, IDC_LISTVIEW);

			if (!width)
				break;

			// resize column width
			_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, NULL, width);

			break;
		}

		case WM_GETMINMAXINFO:
		{
			_r_layout_resizeminimumsize (&layout_manager, lparam);
			break;
		}

		case WM_COMMAND:
		{
			INT ctrl_id = LOWORD (wparam);
			INT notify_code = HIWORD (wparam);

			if (notify_code == 0 && ctrl_id >= IDX_LANGUAGE && ctrl_id <= IDX_LANGUAGE + (INT)(_r_locale_getcount () + 1))
			{
				HMENU hsubmenu;
				HMENU hmenu;

				hmenu = GetMenu (hwnd);

				if (hmenu)
				{
					hsubmenu = GetSubMenu (hmenu, 1);

					if (hsubmenu)
					{
						hsubmenu = GetSubMenu (hsubmenu, LANG_MENU);

						if (hsubmenu)
							_r_locale_apply (hsubmenu, ctrl_id, IDX_LANGUAGE);
					}
				}

				return FALSE;
			}

			switch (ctrl_id)
			{
				case IDC_CODE_CTL:
				{
					PR_STRING string;
					LONG_PTR pos;

					if (notify_code != EN_CHANGE)
						break;

					string = _r_ctrl_getstring (hwnd, ctrl_id);

					if (string)
					{
						_r_str_trimstring2 (&string->sr, L" \r\n\";", 0);

						pos = _r_ctrl_getselection (hwnd, ctrl_id);

						_r_ctrl_setstring (hwnd, ctrl_id, string->buffer);

						_r_ctrl_setselection (hwnd, ctrl_id, pos);

						_r_obj_dereference (string);
					}

					_app_print (hwnd);

					break;
				}

				case IDM_SETTINGS:
				{
					_r_settings_createwindow (hwnd, &SettingsProc, 0);
					break;
				}

				case IDCANCEL: // process Esc key
				case IDM_EXIT:
				{
					DestroyWindow (hwnd);
					break;
				}

				case IDM_ALWAYSONTOP_CHK:
				{
					BOOLEAN new_val;

					new_val = !_r_config_getboolean (L"AlwaysOnTop", FALSE);

					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, new_val);
					_r_config_setboolean (L"AlwaysOnTop", new_val);

					_r_wnd_top (hwnd, new_val);

					break;
				}

				case IDM_DARKMODE_CHK:
				{
					BOOLEAN new_val;

					new_val = !_r_theme_isenabled ();

					_r_theme_enable (hwnd, new_val);

					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, new_val);

					break;
				}

				case IDM_INSERTBUFFER_CHK:
				{
					BOOLEAN new_val;

					new_val = !_r_config_getboolean (L"InsertBufferAtStartup", FALSE);

					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, new_val);
					_r_config_setboolean (L"InsertBufferAtStartup", new_val);

					break;
				}

				case IDM_CHECKUPDATES_CHK:
				{
					BOOLEAN new_val;

					new_val = !_r_update_isenabled (FALSE);

					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, new_val);
					_r_update_enable (new_val);

					break;
				}

				case IDM_WEBSITE:
				{
					_r_shell_opendefault (_r_app_getwebsite_url ());
					break;
				}

				case IDM_CHECKUPDATES:
				{
					_r_update_check (hwnd);
					break;
				}

				case IDM_ABOUT:
				{
					_r_show_aboutmessage (hwnd);
					break;
				}

				case IDM_ZOOM:
				{
					ShowWindow (hwnd, IsZoomed (hwnd) ? SW_RESTORE : SW_MAXIMIZE);
					break;
				}

				case IDM_EXPLORE:
				{
					ULONG_PTR module_hash;
					INT item_id;

					item_id = _r_listview_getselecteditem (hwnd, IDC_LISTVIEW);

					if (item_id == -1)
						break;

					module_hash = _r_listview_getitemlparam (hwnd, IDC_LISTVIEW, item_id);

					_app_opendirectory (hwnd, module_hash);

					break;
				}

				case IDM_COPY:
				{
					PR_STRING string;
					INT item_id;

					item_id = _r_listview_getselecteditem (hwnd, IDC_LISTVIEW);

					if (item_id == -1)
						break;

					string = _r_listview_getitemtext (hwnd, IDC_LISTVIEW, item_id, 0);

					if (string)
					{
						_r_clipboard_set (hwnd, &string->sr);

						_r_obj_dereference (string);
					}

					break;
				}
			}

			break;
		}
	}

	return FALSE;
}

INT APIENTRY wWinMain (
	_In_ HINSTANCE hinst,
	_In_opt_ HINSTANCE prev_hinst,
	_In_ LPWSTR cmdline,
	_In_ INT show_cmd
)
{
	HWND hwnd;

	if (!_r_app_initialize (NULL))
		return ERROR_APP_INIT_FAILURE;

	hwnd = _r_app_createwindow (hinst, MAKEINTRESOURCEW (IDD_MAIN), MAKEINTRESOURCEW (IDI_MAIN), &DlgProc);

	if (!hwnd)
		return ERROR_APP_INIT_FAILURE;

	return _r_wnd_message_callback (hwnd, MAKEINTRESOURCEW (IDA_MAIN));
}
