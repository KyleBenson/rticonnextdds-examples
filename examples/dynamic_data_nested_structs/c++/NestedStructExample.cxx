/* NestedStructExample.cxx
    
    This example 
      - creates the type code of for a nested struct 
            (with inner and outer structs) 
      - creates a DynamicData instance
      - sets the values of the inner struct
      - see the differents between set_complex_member and bind_complex_member
   
    Example:
        
       To run the example application:
                                  
       On Unix: 
       
       objs/<arch>/NestedStructExample 
                            
       On Windows:
       
       objs\<arch>\NestedStructExample  
*/

#include <iostream>
#include <ndds_cpp.h>

using namespace std;

DDS_TypeCode *
inner_struct_get_typecode() {
	static DDS_TypeCode *tc = NULL;
    DDS_TypeCodeFactory* tcf;
	struct DDS_StructMemberSeq members;
	DDS_ExceptionCode_t err;

    /* Getting a reference to the type code factory */	
	tcf = DDS_TypeCodeFactory::get_instance();
    if (tcf == NULL) {
        cerr << "! Unable to get type code factory singleton" << endl;
        goto fail;
    }

    /* First, we create the typeCode for a struct */
    tc = tcf->create_struct_tc("InnerStruct", members, err);
    if (err != DDS_NO_EXCEPTION_CODE) {
        cerr << "! Unable to create struct TC" << endl;
        goto fail;
    }

    /* Case 1 will be a double named x */
    tc->add_member("x", DDS_TYPECODE_MEMBER_ID_INVALID,
            DDS_TypeCodeFactory_get_primitive_tc(tcf, DDS_TK_DOUBLE),
            DDS_TYPECODE_NONKEY_REQUIRED_MEMBER, err);
    if (err != DDS_NO_EXCEPTION_CODE) {
         cerr << "! Unable to add member x" << endl;
        goto fail;
    }

    tc->add_member("y", DDS_TYPECODE_MEMBER_ID_INVALID,
            DDS_TypeCodeFactory_get_primitive_tc(tcf, DDS_TK_DOUBLE),
            DDS_TYPECODE_NONKEY_REQUIRED_MEMBER, err);
    if (err != DDS_NO_EXCEPTION_CODE) {
        cerr << "! Unable to add member y" << endl;
        goto fail;
    }

    DDS_StructMemberSeq_finalize(&members);
    return tc;

fail:
    if (tc != NULL) {
        tcf->delete_tc(tc, err);
    }
    DDS_StructMemberSeq_finalize(&members);
    return NULL;
}

DDS_TypeCode *
outer_struct_get_typecode() {
	static DDS_TypeCode *tc = NULL;
	DDS_TypeCodeFactory *tcf = NULL;
	struct DDS_StructMemberSeq members;
	DDS_ExceptionCode_t err;

    /* Getting a reference to the type code factory */
    tcf = DDS_TypeCodeFactory::get_instance();
     if (tcf == NULL) {
        cerr << "! Unable to get type code factory singleton" << endl;
        goto fail;
    }

    /* First, we create the typeCode for a struct */
    tc = tcf->create_struct_tc("OuterStruct", members, err);
    if (err != DDS_NO_EXCEPTION_CODE) {
         cerr << "! Unable to create struct TC" << endl;
        goto fail;
    }
    
    /* This struct just will have a data named inner, type: struct inner */
    tc->add_member("inner", DDS_TYPECODE_MEMBER_ID_INVALID,
				inner_struct_get_typecode(),
				DDS_TYPECODE_NONKEY_REQUIRED_MEMBER, err);
    if (err != DDS_NO_EXCEPTION_CODE) {
         cerr << "! Unable to add member inner struct" << endl;
        goto fail;
    }

	DDS_StructMemberSeq_finalize(&members);
    return tc;

fail:
    if (tc != NULL) {
        tcf->delete_tc(tc, err);
    }
    DDS_StructMemberSeq_finalize(&members);
    return NULL;
}

int main() {
	struct DDS_TypeCode *inner_tc = inner_struct_get_typecode();
	struct DDS_TypeCode *outer_tc = outer_struct_get_typecode();
	
    DDS_ExceptionCode_t err;
    DDS_ReturnCode_t retcode;
    int ret = -1;

	DDS_DynamicData outer_data(outer_tc, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
	DDS_DynamicData inner_data(inner_tc, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
	DDS_DynamicData bounded_data(NULL,DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);

	cout << " Connext Dynamic Data Nested Struct Example" << endl
		 << "--------------------------------------------" << endl;

	cout << " Data Types" << endl
         << "------------------" << endl;
	inner_tc->print_IDL(0, err);
	outer_tc->print_IDL(0, err);

	/* Setting the inner data */
	retcode = inner_data.set_double("x", 
        DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, 3.14159);
    if (retcode != DDS_RETCODE_OK) {
        cerr << "! Unable to set value 'x' in the inner struct" << endl;
        goto fail;
    }

	retcode = inner_data.set_double("y",
        DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, 2.71828);
    if (retcode != DDS_RETCODE_OK) {
        cerr << "! Unable to set value 'y' in the inner struct" << endl;
        goto fail;
    }

	printf("\n\n get/set_complex_member API\n"
			"------------------\n");
	/* Get/Set complex member API */
	printf("Setting the initial values of struct with set_complex_member()\n");
	retcode = outer_data.set_complex_member("inner",
			DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, inner_data);
    if (retcode != DDS_RETCODE_OK) {
        cerr << "! Unable to set complex struct value " <<
            "(member inner in the outer struct)" << endl; 
        goto fail;
    }

	outer_data.print(stdout, 1);

	retcode = inner_data.clear_all_members();
    if (retcode != DDS_RETCODE_OK) {
        cerr << "! Unable to clear all member in the inner struct" << endl;
        goto fail;
    }

	printf("\n + get_complex_member() called\n");
	
    retcode = outer_data.get_complex_member(inner_data, "inner",
			DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
    if (retcode != DDS_RETCODE_OK) {
        cerr << "! Unable to get complex struct value" <<
            "(member inner in the outer struct)" << endl;
        goto fail;
    }
	printf("\n + inner struct value\n");
	inner_data.print(stdout, 1);

	/* get complex member makes a copy of the member, so modifying the dynamic 
	 data obtained by get complex member WILL NOT modify the outer data */
	printf("\n + setting new values to inner struct\n");
	retcode = inner_data.set_double("x",
        DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, 1.00000);
    if (retcode != DDS_RETCODE_OK) {
        cerr << "! Unable to set value 'x' in the inner struct" << endl;
        goto fail;
    }

	retcode = inner_data.set_double("y", 
        DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, 0.00001);
    if (retcode != DDS_RETCODE_OK) {
        cerr << "! Unable to set value 'y' in the inner struct" << endl;
        goto fail;
    }

	/* Current value of outer_data
	 outer:
	 inner:
	 x: 3.141590
	 y: 2.718280
	 */
	printf("\n + current outer struct value \n");
	outer_data.print(stdout, 1);

	printf("\n\n bind/unbind API\n"
			"------------------\n");
	/* Bind/Unbind member API */

	printf("\n + bind complex member called\n");
	retcode = outer_data.bind_complex_member(bounded_data, "inner",
			DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
    if (retcode != DDS_RETCODE_OK) {
        cerr << "! Unable to bind the structs" << endl;
        goto fail;
    }

	bounded_data.print(stdout, 1);

	/* binding a member does not copy, so modifying the bounded member WILL 
       modify the outer object */
	printf("\n + setting new values to inner struct\n");
	retcode = bounded_data.set_double("x",
			DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, 1.00000);
    if (retcode != DDS_RETCODE_OK) {
        cerr << "! Unable to set value to 'x' with the bounded data" << endl;
        goto fail;
    }

	retcode = bounded_data.set_double("y",
			DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, 0.00001);
    if (retcode != DDS_RETCODE_OK) {
        cerr << "! Unable to set value to 'y' in the bounded data" << endl;
        goto fail;
    }

	/* Current value of outer data
	 outer:
	 inner:
	 x: 1.000000
	 y: 0.000010
	 */

	bounded_data.print(stdout, 1);

	retcode = outer_data.unbind_complex_member(bounded_data);
    if (retcode != DDS_RETCODE_OK) {
        cerr << "! Unable to unbind the data" << endl;
        goto fail;
    }

	printf("\n + current outer struct value \n");
	outer_data.print(stdout, 1);

    ret = 1;
    
fail:
    if (inner_tc != NULL) {
        delete inner_tc;
    }
    
    if (outer_tc != NULL) {
        delete outer_tc;
    }
 	return 0;
}
