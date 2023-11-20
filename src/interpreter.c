/*
interpreter.c - Une
Modified 2023-11-20
*/

/* Header-specific includes. */
#include "interpreter.h"

/* Implementation-specific includes. */
#include <string.h>
#include <math.h>
#include "tools.h"
#include "types/context.h"
#include "datatypes/datatypes.h"

/*
Interpreter function lookup table.
*/
une_interpreter__(*interpreter_table__[]) = {
	&une_interpret_void,
	&une_interpret_int,
	&une_interpret_flt,
	&une_interpret_str,
	&une_interpret_list,
	&une_interpret_object,
	&une_interpret_function,
	&une_interpret_builtin,
	&une_interpret_stmts,
	&une_interpret_cop,
	&une_interpret_not,
	&une_interpret_and,
	&une_interpret_or,
	&une_interpret_nullish,
	&une_interpret_equ,
	&une_interpret_neq,
	&une_interpret_geq,
	&une_interpret_gtr,
	&une_interpret_leq,
	&une_interpret_lss,
	&une_interpret_add,
	&une_interpret_sub,
	&une_interpret_mul,
	&une_interpret_fdiv,
	&une_interpret_div,
	&une_interpret_mod,
	&une_interpret_pow,
	&une_interpret_neg,
	&une_interpret_seek,
	&une_interpret_idx_seek,
	&une_interpret_member_seek,
	&une_interpret_assign,
	&une_interpret_assign_add,
	&une_interpret_assign_sub,
	&une_interpret_assign_pow,
	&une_interpret_assign_mul,
	&une_interpret_assign_fdiv,
	&une_interpret_assign_div,
	&une_interpret_assign_mod,
	&une_interpret_call,
	&une_interpret_for_range,
	&une_interpret_for_element,
	&une_interpret_while,
	&une_interpret_if,
	&une_interpret_assert,
	&une_interpret_continue,
	&une_interpret_break,
	&une_interpret_return,
	&une_interpret_exit,
	&une_interpret_any,
	&une_interpret_all,
	&une_interpret_cover,
	&une_interpret_concatenate,
	&une_interpret_this,
};

/*
*** Interface.
*/

une_result une_interpret(une_node *node)
{
	assert(UNE_NODE_TYPE_IS_IN_LUT(node->type));
	
	LOGINTERPRET_BEGIN(node);
	
	une_result result = interpreter_table__[(node->type)-UNE_R_BGN_LUT_NODES](node);
	assert(UNE_RESULT_TYPE_IS_VALID(result.type));
	
	LOGINTERPRET_END(node);
	
	return result;
}

/*
*** Interpreter table.
*/

une_interpreter__(une_interpret_void)
{
	return (une_result){
		.type = UNE_RT_VOID,
		.value._int = node->content.value._int
	};
}

une_interpreter__(une_interpret_int)
{
	return (une_result){
		.type = UNE_RT_INT,
		.value._int = node->content.value._int
	};
}

une_interpreter__(une_interpret_flt)
{
	return (une_result){
		.type = UNE_RT_FLT,
		.value._flt = node->content.value._flt
	};
}

une_interpreter__(une_interpret_str)
{
	/* DOC: Memory Management: Here we can see that results DUPLICATE strings. */
	une_result result = {
		.type = UNE_RT_STR,
		.value._wcs = wcsdup(node->content.value._wcs)
	};
	verify(result.value._wcs);
	return result;
}

une_interpreter__(une_interpret_list)
{
	UNE_UNPACK_NODE_LIST(node, list, list_size);

	/* Create une_result list. */
	une_result *result_list = une_result_list_create(list_size);

	/* Populate une_result list. */
	UNE_FOR_NODE_LIST_ITEM(i, list_size) {
		result_list[i] = une_result_dereference(une_interpret(list[i]));
		if (result_list[i].type == UNE_RT_ERROR) {
			une_result result = une_result_copy(result_list[i]);
			for (size_t j=0; j<=i; j++)
				une_result_free(result_list[j]);
			free(result_list);
			return result;
		}
	}
	return (une_result){
		.type = UNE_RT_LIST,
		.value._vp = (void*)result_list,
	};
}

une_interpreter__(une_interpret_object)
{
	UNE_UNPACK_NODE_LIST(node, list, list_size);

	/* Create object. */
	une_object *object = malloc(sizeof(*object));
	verify(object);
	object->members_length = list_size;
	object->members = malloc(object->members_length*sizeof(*object->members));
	verify(object->members);
	object->owner = felix->is.context;

	/* Store associations. */
	UNE_FOR_NODE_LIST_ITEM(i, list_size) {
		/* Add association. */
		une_association *association = une_association_create();
		object->members[i-1] = association;
		/* Populate association. */
		association->name = wcsdup(list[i]->content.branch.a->content.value._wcs);
		verify(association->name);
		association->content = une_result_dereference(une_interpret(list[i]->content.branch.b));
		if (association->content.type == UNE_RT_ERROR) {
			une_result result = une_result_copy(association->content);
			UNE_FOR_NODE_LIST_ITEM(j, i)
				une_association_free(object->members[j-1]);
			free(object->members);
			free(object);
			return result;
		}
	}
	
	return (une_result){
		.type = UNE_RT_OBJECT,
		.value._vp = (void*)object
	};
}

une_interpreter__(une_interpret_function)
{
	/* Reduce parameter nodes to a vector of strings. */
	UNE_UNPACK_NODE_LIST(node->content.branch.a, params_n, params_count);
	wchar_t **params = NULL;
	if (params_count > 0) {
		params = malloc(params_count*sizeof(*params));
		verify(params);
	}
	for (size_t i=0; i<params_count; i++) {
		params[i] = wcsdup(params_n[i+1]->content.value._wcs);
		verify(params[i]);
	}
	
	/* Register callable. */
	char *definition_file = NULL;
	if (node->content.branch.c) {
		definition_file = strdup((char*)node->content.branch.c);
		verify(definition_file);
	}

	une_callable *callable = une_callables_add_callable(&felix->is.callables);
	assert(callable);

	callable->definition_file = definition_file;
	callable->definition_point = node->pos;
	callable->params_count = params_count;
	callable->params = params;
	callable->body = une_node_copy(node->content.branch.b);

	/* Return FUNCTION result. */
	return (une_result){
		.type = UNE_RT_FUNCTION,
		.value._id = callable->id
	};
}

une_interpreter__(une_interpret_builtin)
{
	return (une_result){
		.type = UNE_RT_BUILTIN,
		.value._int = node->content.value._int
	};
}

une_interpreter__(une_interpret_stmts)
{
	une_holding old_holding = une_interpreter_state_holding_strip(&felix->is);
	
	une_result result_ = une_result_create(UNE_RT_VOID);

	UNE_UNPACK_NODE_LIST(node, nodes, nodes_size);

	UNE_FOR_NODE_LIST_ITEM(i, nodes_size) {
		/* Free previous result. */
		une_result_free(result_);
		
		/* Interpret statement. */
		result_ = une_interpret(nodes[i]);
		if (!une_result_is_reference_to_foreign_object(&felix->is, result_))
			result_ = une_result_dereference(result_);
		
		/* Drop held results. */
		une_interpreter_state_holding_purge(&felix->is);
		
		/* Break if required. */
		if (result_.type == UNE_RT_ERROR || result_.type == UNE_RT_CONTINUE || result_.type == UNE_RT_BREAK || felix->is.should_return || felix->is.should_exit)
			break;
	}
	
	une_interpreter_state_holding_reinstate(&felix->is, old_holding);
	
	return result_; /* Return last result. */
}

une_interpreter__(une_interpret_cop)
{
	/* Evaluate condition. */
	une_result condition = une_result_dereference(une_interpret(node->content.branch.a));
	if (condition.type == UNE_RT_ERROR)
		return condition;
	
	/* Check if condition applies. */
	une_int is_true = une_result_is_true(condition);
	une_result_free(condition);

	/* Evaluate correct branch. */
	if (is_true)
		return une_result_dereference(une_interpret(node->content.branch.b));
	return une_result_dereference(une_interpret(node->content.branch.c));
}

une_interpreter__(une_interpret_not)
{
	/* Evaluate expression. */
	une_result center = une_result_dereference(une_interpret(node->content.branch.a));
	if (center.type == UNE_RT_ERROR)
		return center;
	
	/* Check truth of result. */
	une_int is_true = une_result_is_true(center);
	une_result_free(center);
	
	return (une_result){
		.type = UNE_RT_INT,
		.value._int = !is_true
	};
}

une_interpreter__(une_interpret_and)
{
	/* Check if branch A is true. */
	une_result left = une_result_dereference(une_interpret(node->content.branch.a));
	if (left.type == UNE_RT_ERROR || !une_result_is_true(left))
		return left;
	une_result_free(left);

	/* Now that we checked branch A, branch B will always hold the outcome of this function. */
	return une_result_dereference(une_interpret(node->content.branch.b));
}

une_interpreter__(une_interpret_or)
{
	/* Check if branch A is true. */
	une_result left = une_result_dereference(une_interpret(node->content.branch.a));
	if (left.type == UNE_RT_ERROR || une_result_is_true(left))
		return left;
	une_result_free(left);

	/* Now that we checked branch A, branch B will always hold the outcome of this function. */
	return une_result_dereference(une_interpret(node->content.branch.b));
}

une_interpreter__(une_interpret_nullish)
{
	/* Check if branch A is not VOID. */
	une_result left = une_result_dereference(une_interpret(node->content.branch.a));
	if (left.type != UNE_RT_VOID)
		return left;
	une_result_free(left);

	/* Now that we checked branch A, branch B will always hold the outcome of this function. */
	return une_result_dereference(une_interpret(node->content.branch.b));
}

une_interpreter__(une_interpret_equ)
{
	return une_interpret_comparison(node, &une_result_equ_result);
}

une_interpreter__(une_interpret_neq)
{
	return une_interpret_comparison(node, &une_result_neq_result);
}

une_interpreter__(une_interpret_gtr)
{
	return une_interpret_comparison(node, &une_result_gtr_result);
}

une_interpreter__(une_interpret_geq)
{
	return une_interpret_comparison(node, &une_result_geq_result);
}

une_interpreter__(une_interpret_lss)
{
	return une_interpret_comparison(node, &une_result_lss_result);
}

une_interpreter__(une_interpret_leq)
{
	return une_interpret_comparison(node, &une_result_leq_result);
}

une_interpreter__(une_interpret_add)
{
	/* Evalute branches. */
	une_result left = une_result_dereference(une_interpret(node->content.branch.a));
	if (left.type == UNE_RT_ERROR)
		return left;
	une_result right = une_result_dereference(une_interpret(node->content.branch.b));
	if (right.type == UNE_RT_ERROR) {
		une_result_free(left);
		return right;
	}
	
	une_result sum = une_result_create(UNE_RT_none__);
	une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
	if (dt_left.add != NULL)
		sum = dt_left.add(left, right);
	une_result_free(left);
	une_result_free(right);
	if (dt_left.add == NULL || sum.type == UNE_RT_ERROR) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
		sum = une_result_create(UNE_RT_ERROR);
	}
	return sum;
}

une_interpreter__(une_interpret_sub)
{
	/* Evaluate branches. */
	une_result left = une_result_dereference(une_interpret(node->content.branch.a));
	if (left.type == UNE_RT_ERROR)
		return left;
	une_result right = une_result_dereference(une_interpret(node->content.branch.b));
	if (right.type == UNE_RT_ERROR) {
		une_result_free(left);
		return right;
	}
	
	une_result difference = une_result_create(UNE_RT_none__);
	une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
	if (dt_left.sub != NULL)
		difference = dt_left.sub(left, right);
	une_result_free(left);
	une_result_free(right);
	if (dt_left.sub == NULL || difference.type == UNE_RT_ERROR) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
		difference = une_result_create(UNE_RT_ERROR);
	}
	return difference;
}

une_interpreter__(une_interpret_mul)
{
	/* Evaluate branches. */
	une_result left = une_result_dereference(une_interpret(node->content.branch.a));
	if (left.type == UNE_RT_ERROR)
		return left;
	une_result right = une_result_dereference(une_interpret(node->content.branch.b));
	if (right.type == UNE_RT_ERROR) {
		une_result_free(left);
		return right;
	}
	
	une_result product = une_result_create(UNE_RT_none__);
	une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
	if (dt_left.mul != NULL)
		product = dt_left.mul(left, right);
	une_result_free(left);
	une_result_free(right);
	if (dt_left.mul == NULL || product.type == UNE_RT_ERROR) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
		product = une_result_create(UNE_RT_ERROR);
	}
	return product;
}

une_interpreter__(une_interpret_div)
{
	/* Evaluate branches. */
	une_result left = une_result_dereference(une_interpret(node->content.branch.a));
	if (left.type == UNE_RT_ERROR)
		return left;
	une_result right = une_result_dereference(une_interpret(node->content.branch.b));
	if (right.type == UNE_RT_ERROR) {
		une_result_free(left);
		return right;
	}
	
	une_result quotient = une_result_create(UNE_RT_none__);
	une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
	if (dt_left.div != NULL)
		quotient = dt_left.div(left, right);
	une_result_free(left);
	une_result_free(right);
	if (dt_left.div == NULL || quotient.type == UNE_RT_ERROR) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
		quotient = une_result_create(UNE_RT_ERROR);
	}
	if (quotient.type == UNE_RT_FLT && isinf(quotient.value._flt)) {
		/* Zero division. */
		felix->error = UNE_ERROR_SET(UNE_ET_ZERO_DIVISION, node->pos);
		quotient = une_result_create(UNE_RT_ERROR);
	}
	return quotient;
}

une_interpreter__(une_interpret_fdiv)
{
	/* Evaluate branches. */
	une_result left = une_result_dereference(une_interpret(node->content.branch.a));
	if (left.type == UNE_RT_ERROR)
		return left;
	une_result right = une_result_dereference(une_interpret(node->content.branch.b));
	if (right.type == UNE_RT_ERROR) {
		une_result_free(left);
		return right;
	}
	
	une_result quotient = une_result_create(UNE_RT_none__);
	une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
	if (dt_left.fdiv != NULL)
		quotient = dt_left.fdiv(left, right);
	une_result_free(left);
	une_result_free(right);
	if (dt_left.fdiv == NULL || quotient.type == UNE_RT_ERROR) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
		quotient = une_result_create(UNE_RT_ERROR);
	}
	if (quotient.type == UNE_RT_FLT && isinf(quotient.value._flt)) {
		/* Zero division. */
		felix->error = UNE_ERROR_SET(UNE_ET_ZERO_DIVISION, node->pos);
		quotient = une_result_create(UNE_RT_ERROR);
	}
	return quotient;
}

une_interpreter__(une_interpret_mod)
{
	/* Evaluate branches. */
	une_result left = une_result_dereference(une_interpret(node->content.branch.a));
	if (left.type == UNE_RT_ERROR)
		return left;
	une_result right = une_result_dereference(une_interpret(node->content.branch.b));
	if (right.type == UNE_RT_ERROR) {
		une_result_free(left);
		return right;
	}
	
	une_result remainder = une_result_create(UNE_RT_none__);
	une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
	if (dt_left.mod != NULL)
		remainder = dt_left.mod(left, right);
	une_result_free(left);
	une_result_free(right);
	if (dt_left.mod == NULL || remainder.type == UNE_RT_ERROR) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
		remainder = une_result_create(UNE_RT_ERROR);
	}
	return remainder;
}

une_interpreter__(une_interpret_pow)
{
	/* Evaluate branches. */
	une_result left = une_result_dereference(une_interpret(node->content.branch.a));
	if (left.type == UNE_RT_ERROR)
		return left;
	une_result right = une_result_dereference(une_interpret(node->content.branch.b));
	if (right.type == UNE_RT_ERROR) {
		une_result_free(left);
		return right;
	}
	
	une_result raised = une_result_create(UNE_RT_none__);
	une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
	if (dt_left.pow != NULL)
		raised = dt_left.pow(left, right);
	une_result_free(left);
	une_result_free(right);
	if (dt_left.pow == NULL || raised.type == UNE_RT_ERROR) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
		raised = une_result_create(UNE_RT_ERROR);
	}
	if (raised.type == UNE_RT_FLT && isnan(raised.value._flt)) {
		/* Unreal number. */
		felix->error = UNE_ERROR_SET(UNE_ET_UNREAL_NUMBER, node->pos);
		raised = une_result_create(UNE_RT_ERROR);
	}
	return raised;
}

une_interpreter__(une_interpret_neg)
{
	/* Evaluate branch. */
	une_result center = une_result_dereference(une_interpret(node->content.branch.a));
	if (center.type == UNE_RT_ERROR)
		return center;
	
	une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(center);
	if (dt_left.negate == NULL) {
		une_result_free(center);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	une_result negative = dt_left.negate(center);
	une_result_free(center);
	return negative;
}

une_interpreter__(une_interpret_seek)
{
	return une_interpret_seek_or_create(node, true);
}

une_interpreter__(une_interpret_idx_seek)
{
	if (node->content.branch.c)
		return une_interpret_idx_seek_range(node);
	return une_interpret_idx_seek_index(node);
}

une_interpreter__(une_interpret_member_seek)
{
	/* Evaluate subject. */
	une_result subject = une_interpret(node->content.branch.a);
	if (subject.type == UNE_RT_ERROR)
		return subject;
	if (subject.type != UNE_RT_REFERENCE && subject.type != UNE_RT_OBJECT) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
		une_result_free(subject);
		return une_result_create(UNE_RT_ERROR);
	}
	if (subject.type == UNE_RT_OBJECT) {
		assert(felix->is.holding.buffer);
		une_result *object_container = une_interpreter_state_holding_add(&felix->is, subject);
		subject = (une_result){
			.type = UNE_RT_REFERENCE,
			.reference = (une_reference){
				.type = UNE_FT_SINGLE,
				.root = (void*)object_container
			}
		};
	}
	
	/* Get applicable datatype. */
	une_datatype dt_result = UNE_DATATYPE_FOR_RESULT(subject);
	
	/* Check if subject supports members. */
	if (!dt_result.refer_to_member) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
		une_result_free(subject);
		return une_result_create(UNE_RT_ERROR);
	}
	assert(dt_result.member_exists);
	
	/* Extract member name. */
	assert(node->content.branch.b->type == UNE_NT_NAME);
	wchar_t *name = node->content.branch.b->content.value._wcs;
	
	/* Refer to member. */
	if (!dt_result.member_exists(subject, name)) {
		felix->error = UNE_ERROR_SET(UNE_ET_SYMBOL_NOT_DEFINED, node->content.branch.b->pos);
		une_result_free(subject);
		return une_result_create(UNE_RT_ERROR);
	}
	une_result member = dt_result.refer_to_member(subject, name);
	assert(member.type == UNE_RT_REFERENCE);
	
	/* Register container as 'this' contestant. */
	une_result_free(felix->is.this_contestant);
	felix->is.this_contestant = subject;
	
	return member;
}

une_interpreter__(une_interpret_assign)
{
	/* Evaluate value. */
	une_result value = une_result_dereference(une_interpret(node->content.branch.b));
	if (value.type == UNE_RT_ERROR)
		return value;
	
	/* Evaluate assignee. */
	une_result assignee;
	if (node->content.branch.a->type == UNE_NT_SEEK)
		assignee = une_interpret_seek_or_create(node->content.branch.a, false);
	else
		assignee = une_interpret(node->content.branch.a);
	if (assignee.type == UNE_RT_ERROR) {
		une_result_free(value);
		return assignee;
	}
	
	/* Get applicable datatype. */
	une_datatype dt_assignee = UNE_DATATYPE_FOR_RESULT(assignee);
	
	/* Check if value can be assigned. If .can_assign is undefined, assume any value can be assigned. */
	if (dt_assignee.can_assign && !dt_assignee.can_assign(assignee.reference, value)) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
		une_result_free(assignee);
		une_result_free(value);
		return une_result_create(UNE_RT_ERROR);
	}
	
	/* Assign value. If .assign is undefined, assume reference.root points to a generic une_result*. */
	if (dt_assignee.assign) {
		dt_assignee.assign(assignee.reference, value);
		une_result_free(value);
	} else {
		une_result *root = (une_result*)assignee.reference.root;
		une_result_free(*root); /* Free the value currently stored at reference.root. */
		*root = value; /* Instead of copying this result, we just use the original; this way, we also don't need to worry about freeing it. */
	}
	une_result_free(assignee);
	
	return une_result_create(UNE_RT_VOID);
}

une_interpreter__(une_interpret_assign_add)
{
	/* Evaluate assignee. */
	une_result assignee = une_interpret(node->content.branch.a);
	if (assignee.type == UNE_RT_ERROR)
		return assignee;
	
	/* Access subject. */
	une_result *subject;
	if (assignee.type == UNE_RT_REFERENCE) {
		if (assignee.reference.type != UNE_FT_SINGLE) {
			une_result_free(assignee);
			felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
			return une_result_create(UNE_RT_ERROR);
		}
		subject = (une_result*)assignee.reference.root;
	} else {
		assert(UNE_RESULT_TYPE_IS_DATA_TYPE(assignee.type));
		subject = &assignee;
	}
	
	/* Get applicable datatype. */
	une_datatype dt_subject = UNE_DATATYPE_FOR_RESULT(*subject);
	
	/* Check if operation is possible. */
	if (!dt_subject.add) {
		une_result_free(assignee);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	
	/* Evaluate operand. */
	une_result operand = une_result_dereference(une_interpret(node->content.branch.b));
	if (operand.type == UNE_RT_ERROR) {
		une_result_free(assignee);
		return operand;
	}
	
	/* Perform operation. */
	une_result result = dt_subject.add(*subject, operand);
	if (result.type == UNE_RT_ERROR) {
		une_result_free(assignee);
		une_result_free(operand);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
		return result;
	}
	une_result_free(assignee);
	une_result_free(operand);
	une_result_free(*subject); /* Free existing result. */
	*subject = result; /* Instead of copying this result, we just use the original; this way, we also don't need to worry about freeing it. */
	
	return une_result_create(UNE_RT_VOID);
}

une_interpreter__(une_interpret_assign_sub)
{
	/* Evaluate assignee. */
	une_result assignee = une_interpret(node->content.branch.a);
	if (assignee.type == UNE_RT_ERROR)
		return assignee;
	
	/* Access subject. */
	une_result *subject;
	if (assignee.type == UNE_RT_REFERENCE) {
		if (assignee.reference.type != UNE_FT_SINGLE) {
			une_result_free(assignee);
			felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
			return une_result_create(UNE_RT_ERROR);
		}
		subject = (une_result*)assignee.reference.root;
	} else {
		assert(UNE_RESULT_TYPE_IS_DATA_TYPE(assignee.type));
		subject = &assignee;
	}
	
	/* Get applicable datatype. */
	une_datatype dt_subject = UNE_DATATYPE_FOR_RESULT(*subject);
	
	/* Check if operation is possible. */
	if (!dt_subject.sub) {
		une_result_free(assignee);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	
	/* Evaluate operand. */
	une_result operand = une_result_dereference(une_interpret(node->content.branch.b));
	if (operand.type == UNE_RT_ERROR) {
		une_result_free(assignee);
		return operand;
	}
	
	/* Perform operation. */
	une_result result = dt_subject.sub(*subject, operand);
	if (result.type == UNE_RT_ERROR) {
		une_result_free(assignee);
		une_result_free(operand);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
		return result;
	}
	une_result_free(assignee);
	une_result_free(operand);
	une_result_free(*subject); /* Free existing result. */
	*subject = result; /* Instead of copying this result, we just use the original; this way, we also don't need to worry about freeing it. */
	
	return une_result_create(UNE_RT_VOID);
}

une_interpreter__(une_interpret_assign_pow)
{
	/* Evaluate assignee. */
	une_result assignee = une_interpret(node->content.branch.a);
	if (assignee.type == UNE_RT_ERROR)
		return assignee;
	
	/* Access subject. */
	une_result *subject;
	if (assignee.type == UNE_RT_REFERENCE) {
		if (assignee.reference.type != UNE_FT_SINGLE) {
			une_result_free(assignee);
			felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
			return une_result_create(UNE_RT_ERROR);
		}
		subject = (une_result*)assignee.reference.root;
	} else {
		assert(UNE_RESULT_TYPE_IS_DATA_TYPE(assignee.type));
		subject = &assignee;
	}
	
	/* Get applicable datatype. */
	une_datatype dt_subject = UNE_DATATYPE_FOR_RESULT(*subject);
	
	/* Check if operation is possible. */
	if (!dt_subject.pow) {
		une_result_free(assignee);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	
	/* Evaluate operand. */
	une_result operand = une_result_dereference(une_interpret(node->content.branch.b));
	if (operand.type == UNE_RT_ERROR) {
		une_result_free(assignee);
		return operand;
	}
	
	/* Perform operation. */
	une_result result = dt_subject.pow(*subject, operand);
	if (result.type == UNE_RT_ERROR) {
		une_result_free(assignee);
		une_result_free(operand);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
		return result;
	}
	une_result_free(assignee);
	une_result_free(operand);
	une_result_free(*subject); /* Free existing result. */
	*subject = result; /* Instead of copying this result, we just use the original; this way, we also don't need to worry about freeing it. */
	
	return une_result_create(UNE_RT_VOID);
}

une_interpreter__(une_interpret_assign_mul)
{
	/* Evaluate assignee. */
	une_result assignee = une_interpret(node->content.branch.a);
	if (assignee.type == UNE_RT_ERROR)
		return assignee;
	
	/* Access subject. */
	une_result *subject;
	if (assignee.type == UNE_RT_REFERENCE) {
		if (assignee.reference.type != UNE_FT_SINGLE) {
			une_result_free(assignee);
			felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
			return une_result_create(UNE_RT_ERROR);
		}
		subject = (une_result*)assignee.reference.root;
	} else {
		assert(UNE_RESULT_TYPE_IS_DATA_TYPE(assignee.type));
		subject = &assignee;
	}
	
	/* Get applicable datatype. */
	une_datatype dt_subject = UNE_DATATYPE_FOR_RESULT(*subject);
	
	/* Check if operation is possible. */
	if (!dt_subject.mul) {
		une_result_free(assignee);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	
	/* Evaluate operand. */
	une_result operand = une_result_dereference(une_interpret(node->content.branch.b));
	if (operand.type == UNE_RT_ERROR) {
		une_result_free(assignee);
		return operand;
	}
	
	/* Perform operation. */
	une_result result = dt_subject.mul(*subject, operand);
	if (result.type == UNE_RT_ERROR) {
		une_result_free(assignee);
		une_result_free(operand);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
		return result;
	}
	une_result_free(assignee);
	une_result_free(operand);
	une_result_free(*subject); /* Free existing result. */
	*subject = result; /* Instead of copying this result, we just use the original; this way, we also don't need to worry about freeing it. */
	
	return une_result_create(UNE_RT_VOID);
}

une_interpreter__(une_interpret_assign_fdiv)
{
	/* Evaluate assignee. */
	une_result assignee = une_interpret(node->content.branch.a);
	if (assignee.type == UNE_RT_ERROR)
		return assignee;
	
	/* Access subject. */
	une_result *subject;
	if (assignee.type == UNE_RT_REFERENCE) {
		if (assignee.reference.type != UNE_FT_SINGLE) {
			une_result_free(assignee);
			felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
			return une_result_create(UNE_RT_ERROR);
		}
		subject = (une_result*)assignee.reference.root;
	} else {
		assert(UNE_RESULT_TYPE_IS_DATA_TYPE(assignee.type));
		subject = &assignee;
	}
	
	/* Get applicable datatype. */
	une_datatype dt_subject = UNE_DATATYPE_FOR_RESULT(*subject);
	
	/* Check if operation is possible. */
	if (!dt_subject.fdiv) {
		une_result_free(assignee);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	
	/* Evaluate operand. */
	une_result operand = une_result_dereference(une_interpret(node->content.branch.b));
	if (operand.type == UNE_RT_ERROR) {
		une_result_free(assignee);
		return operand;
	}
	
	/* Perform operation. */
	une_result result = dt_subject.fdiv(*subject, operand);
	if (result.type == UNE_RT_ERROR) {
		une_result_free(assignee);
		une_result_free(operand);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
		return result;
	}
	une_result_free(assignee);
	une_result_free(operand);
	une_result_free(*subject); /* Free existing result. */
	*subject = result; /* Instead of copying this result, we just use the original; this way, we also don't need to worry about freeing it. */
	
	return une_result_create(UNE_RT_VOID);
}

une_interpreter__(une_interpret_assign_div)
{
	/* Evaluate assignee. */
	une_result assignee = une_interpret(node->content.branch.a);
	if (assignee.type == UNE_RT_ERROR)
		return assignee;
	
	/* Access subject. */
	une_result *subject;
	if (assignee.type == UNE_RT_REFERENCE) {
		if (assignee.reference.type != UNE_FT_SINGLE) {
			une_result_free(assignee);
			felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
			return une_result_create(UNE_RT_ERROR);
		}
		subject = (une_result*)assignee.reference.root;
	} else {
		assert(UNE_RESULT_TYPE_IS_DATA_TYPE(assignee.type));
		subject = &assignee;
	}
	
	/* Get applicable datatype. */
	une_datatype dt_subject = UNE_DATATYPE_FOR_RESULT(*subject);
	
	/* Check if operation is possible. */
	if (!dt_subject.div) {
		une_result_free(assignee);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	
	/* Evaluate operand. */
	une_result operand = une_result_dereference(une_interpret(node->content.branch.b));
	if (operand.type == UNE_RT_ERROR) {
		une_result_free(assignee);
		return operand;
	}
	
	/* Perform operation. */
	une_result result = dt_subject.div(*subject, operand);
	if (result.type == UNE_RT_ERROR) {
		une_result_free(assignee);
		une_result_free(operand);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
		return result;
	}
	une_result_free(assignee);
	une_result_free(operand);
	une_result_free(*subject); /* Free existing result. */
	*subject = result; /* Instead of copying this result, we just use the original; this way, we also don't need to worry about freeing it. */
	
	return une_result_create(UNE_RT_VOID);
}

une_interpreter__(une_interpret_assign_mod)
{
	/* Evaluate assignee. */
	une_result assignee = une_interpret(node->content.branch.a);
	if (assignee.type == UNE_RT_ERROR)
		return assignee;
	
	/* Access subject. */
	une_result *subject;
	if (assignee.type == UNE_RT_REFERENCE) {
		if (assignee.reference.type != UNE_FT_SINGLE) {
			une_result_free(assignee);
			felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
			return une_result_create(UNE_RT_ERROR);
		}
		subject = (une_result*)assignee.reference.root;
	} else {
		assert(UNE_RESULT_TYPE_IS_DATA_TYPE(assignee.type));
		subject = &assignee;
	}
	
	/* Get applicable datatype. */
	une_datatype dt_subject = UNE_DATATYPE_FOR_RESULT(*subject);
	
	/* Check if operation is possible. */
	if (!dt_subject.mod) {
		une_result_free(assignee);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	
	/* Evaluate operand. */
	une_result operand = une_result_dereference(une_interpret(node->content.branch.b));
	if (operand.type == UNE_RT_ERROR) {
		une_result_free(assignee);
		return operand;
	}
	
	/* Perform operation. */
	une_result result = dt_subject.mod(*subject, operand);
	if (result.type == UNE_RT_ERROR) {
		une_result_free(assignee);
		une_result_free(operand);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
		return result;
	}
	une_result_free(assignee);
	une_result_free(operand);
	une_result_free(*subject); /* Free existing result. */
	*subject = result; /* Instead of copying this result, we just use the original; this way, we also don't need to worry about freeing it. */
	
	return une_result_create(UNE_RT_VOID);
}

une_interpreter__(une_interpret_call)
{
	/* Interpret arguments. */
	une_result args = une_result_dereference(une_interpret_list(node->content.branch.b));
	if (args.type == UNE_RT_ERROR)
		return args;
	
	/* Get callable. */
	wchar_t *label = NULL;
	if (node->content.branch.a->type == UNE_NT_SEEK && node->content.branch.a->content.branch.a->type == UNE_NT_NAME)
		label = node->content.branch.a->content.branch.a->content.value._wcs;
	une_result callable = une_result_dereference(une_interpret(node->content.branch.a));
	if (callable.type == UNE_RT_ERROR) {
		une_result_free(args);
		return callable;
	}
	
	/* Determine if this is a method call. */
	bool is_method_call = node->content.branch.a->type == UNE_NT_MEMBER_SEEK;
	une_result this_before = une_result_create(UNE_RT_VOID);
	if (is_method_call) {
		/* Protect current 'this'. */
		this_before = felix->is.this;
		/* Promote 'this' contestant to actual 'this'. */
		assert(felix->is.this_contestant.type == UNE_RT_REFERENCE || felix->is.this_contestant.type == UNE_RT_OBJECT);
		felix->is.this = felix->is.this_contestant;
		felix->is.this_contestant = une_result_create(UNE_RT_VOID);
	}
	
	/* Ensure result type is callable. */
	une_datatype dt_callable = UNE_DATATYPE_FOR_RESULT(callable);
	if (dt_callable.call == NULL) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
		une_result_free(args);
		une_result_free(callable);
		return une_result_create(UNE_RT_ERROR);
	}

	/* Execute function. */
	une_result result = dt_callable.call(node, callable, args, label);
	une_result_free(args);
	une_result_free(callable);
	
	/* Free our 'this' and reinstate the previous 'this'. */
	if (is_method_call) {
		une_result_free(felix->is.this);
		felix->is.this = this_before;
	}
	
	return result;
}

une_interpreter__(une_interpret_for_range)
{
	/* Get range. */
	une_result result = une_interpret_as(node->content.branch.b, UNE_RT_INT);
	if (result.type == UNE_RT_ERROR)
		return result;
	une_int from = result.value._int;
	une_result_free(result);
	result = une_interpret_as(node->content.branch.c, UNE_RT_INT);
	if (result.type == UNE_RT_ERROR)
		return result;
	une_int till = result.value._int;
	une_result_free(result);
	if (from == till)
		return une_result_create(UNE_RT_VOID);
	
	/* Determine step. */
	une_int step;
	if (from < till)
		step = 1;
	else
		step = -1;
	
	/* Get loop variable. */
	wchar_t *name = node->content.branch.a->content.value._wcs;
	une_association *var = une_variable_find_or_create(felix->is.context, name); /* We only check the *local* variables. */
	
	/* Loop. */
	for (une_int i=from; i!=till; i+=step) {
		var = une_variable_find(felix->is.context, name); /* Avoid stale pointer if variable buffer grows. */
		une_result_free(var->content);
		var->content = (une_result){
			.type = UNE_RT_INT,
			.value._int = i
		};
		result = une_result_dereference(une_interpret(node->content.branch.d));
		if (result.type == UNE_RT_ERROR || felix->is.should_return || felix->is.should_exit)
			return result;
		if (result.type == UNE_RT_BREAK) {
			une_result_free(result);
			break;
		}
		une_result_free(result);
	}
	
	return une_result_create(UNE_RT_VOID);
}

une_interpreter__(une_interpret_for_element)
{
	/* Get range. */
	une_result elements = une_result_dereference(une_interpret(node->content.branch.b));
	if (elements.type == UNE_RT_ERROR)
		return elements;
	une_datatype elements_dt = UNE_DATATYPE_FOR_RESULT(elements);
	if (!elements_dt.get_len) {
		une_result_free(elements);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	une_int length = (une_int)elements_dt.get_len(elements);
	assert(elements_dt.refer_to_index);
	
	/* Get loop variable. */
	wchar_t *name = node->content.branch.a->content.value._wcs;
	une_association *var = une_variable_find_or_create(felix->is.context, name); /* We only check the *local* variables. */
	
	/* Prepare internal index. */
	une_result index = une_result_create(UNE_RT_INT);
	index.value._int = 0;
	
	/* Loop. */
	for (; index.value._int<length; index.value._int++) {
		var = une_variable_find(felix->is.context, name); /* Avoid stale pointer if variable buffer grows. */
		une_result_free(var->content);
		var->content = une_result_dereference(elements_dt.refer_to_index(elements, index));
		une_result result = une_result_dereference(une_interpret(node->content.branch.c));
		if (result.type == UNE_RT_ERROR || felix->is.should_return || felix->is.should_exit)
			return result;
		if (result.type == UNE_RT_BREAK) {
			une_result_free(result);
			break;
		}
		une_result_free(result);
	}
	
	une_result_free(elements);
	return une_result_create(UNE_RT_VOID);
}

une_interpreter__(une_interpret_while)
{
	une_result result, condition;
	une_result_type result_type;
	une_int condition_is_true;

	while (true) {
		condition = une_result_dereference(une_interpret(node->content.branch.a));
		if (condition.type == UNE_RT_ERROR)
			return condition;
		condition_is_true = une_result_is_true(condition);
		une_result_free(condition);
		if (!condition_is_true)
			break;
		result = une_result_dereference(une_interpret(node->content.branch.b));
		if (result.type == UNE_RT_ERROR || felix->is.should_return || felix->is.should_exit)
			return result;
		result_type = result.type;
		une_result_free(result);
		if (result_type == UNE_RT_BREAK)
			break;
	}

	return une_result_create(UNE_RT_VOID);
}

une_interpreter__(une_interpret_if)
{
	/* Check if predicate applies. */
	une_result predicate = une_result_dereference(une_interpret(node->content.branch.a));
	if (predicate.type == UNE_RT_ERROR)
		return predicate;
	une_int is_true = une_result_is_true(predicate);
	une_result_free(predicate);
	
	if (is_true)
		return une_result_dereference(une_interpret(node->content.branch.b));
	if (node->content.branch.c != NULL)
		return une_result_dereference(une_interpret(node->content.branch.c));
	return une_result_create(UNE_RT_VOID);
}

une_interpreter__(une_interpret_assert)
{
	/* Check if assertion is met. */
	une_result assertion = une_result_dereference(une_interpret(node->content.branch.a));
	if (assertion.type == UNE_RT_ERROR)
		return assertion;
	une_int is_true = une_result_is_true(assertion);
	une_result_free(assertion);
	
	if (!is_true) {
		felix->error = UNE_ERROR_SET(UNE_ET_ASSERTION_NOT_MET, node->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	return une_result_create(UNE_RT_VOID);
}

une_interpreter__(une_interpret_continue)
{
	return une_result_create(UNE_RT_CONTINUE);
}

une_interpreter__(une_interpret_break)
{
	return une_result_create(UNE_RT_BREAK);
}

une_interpreter__(une_interpret_return)
{
	une_result result;
	if (node->content.branch.a != NULL)
		result = une_interpret(node->content.branch.a); /* Dereferencing happens in interpret_stmts. */
	else
		result = une_result_create(UNE_RT_VOID);
	felix->is.should_return = true;
	return result;
}

une_interpreter__(une_interpret_exit)
{
	une_result result;
	if (node->content.branch.a != NULL)
		result = une_interpret_as(node->content.branch.a, UNE_RT_INT);
	else
		result = une_result_create(UNE_RT_VOID);
	felix->is.should_exit = true;
	return result;
}

une_interpreter__(une_interpret_any)
{
	/* Whenever ANY is allowed, it is handled directly, without using this function.
	Therefore, whenever this function *is* called, it means ANY is not allowed. */
	felix->error = UNE_ERROR_SET(UNE_ET_MISPLACED_ANY_OR_ALL, node->pos);
	return une_result_create(UNE_RT_ERROR);
}

une_interpreter__(une_interpret_all)
{
	/* Whenever ALL is allowed, it is handled directly, without using this function.
	Therefore, whenever this function *is* called, it means ALL is not allowed. */
	felix->error = UNE_ERROR_SET(UNE_ET_MISPLACED_ANY_OR_ALL, node->pos);
	return une_result_create(UNE_RT_ERROR);
}

une_interpreter__(une_interpret_cover)
{
	/* Try to interpret branch A. */
	une_result left = une_result_dereference(une_interpret(node->content.branch.a));
	if (left.type != UNE_RT_ERROR)
		return left;
	felix->error = une_error_create();
	une_result_free(left);

	/* Now that we checked branch A, branch B will always hold the outcome of this function. */
	return une_result_dereference(une_interpret(node->content.branch.b));
}

une_interpreter__(une_interpret_concatenate)
{
	/* Evalute branches. */
	une_result left = une_result_dereference(une_interpret(node->content.branch.a));
	if (left.type == UNE_RT_ERROR)
		return left;
	une_result right = une_result_dereference(une_interpret(node->content.branch.b));
	if (right.type == UNE_RT_ERROR) {
		une_result_free(left);
		return right;
	}
	
	/* Convert both branches to strings and add them. */
	une_result concatenated;
	une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
	une_datatype dt_right = UNE_DATATYPE_FOR_RESULT(right);
	if (!dt_left.as_str) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
		concatenated = une_result_create(UNE_RT_ERROR);
	} else if (!dt_right.as_str) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
		concatenated = une_result_create(UNE_RT_ERROR);
	} else {
		une_result left_as_str = dt_left.as_str(left);
		une_result right_as_str = dt_right.as_str(right);
		concatenated = une_datatype_str_add(left_as_str, right_as_str);
		une_result_free(left_as_str);
		une_result_free(right_as_str);
	}
	
	une_result_free(left);
	une_result_free(right);
	return concatenated;
}

une_interpreter__(une_interpret_this)
{
	return felix->is.this;
}

/*
*** Helpers.
*/

une_interpreter__(une_interpret_as, une_result_type type)
{
	une_result result = une_result_dereference(une_interpret(node));
	if (result.type != type && result.type != UNE_RT_ERROR) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
		une_result_free(result);
		result = une_result_create(UNE_RT_ERROR);
	}
	return result;
}

une_interpreter__(une_interpret_seek_or_create, bool existing_only)
{
	/* Extract information. */
	wchar_t *name = node->content.branch.a->content.value._wcs;
	bool global = (une_node*)node->content.branch.b;
	
	/* Find variable. */
	une_association *var;
	if (global) {
		if (existing_only)
			var = une_variable_find_global(felix->is.context, name);
		else
			var = une_variable_find_or_create_global(felix->is.context, name);
	} else {
		if (existing_only)
			var = une_variable_find(felix->is.context, name);
		else
			var = une_variable_find_or_create(felix->is.context, name);
	}
	if (var == NULL) {
		felix->error = UNE_ERROR_SET(UNE_ET_SYMBOL_NOT_DEFINED, node->content.branch.a->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	
	/* Return reference to variable content. */
	return (une_result){
		.type = UNE_RT_REFERENCE,
		.reference = (une_reference){
			.type = UNE_FT_SINGLE,
			.root = (void*)&var->content
		}
	};
}

une_interpreter__(une_interpret_idx_seek_index)
{
	/* Evaluate subject. */
	une_result subject = une_interpret(node->content.branch.a);
	if (subject.type == UNE_RT_ERROR)
		return subject;
	
	/* Get applicable datatype. */
	une_datatype dt_result = UNE_DATATYPE_FOR_RESULT(subject);
	
	/* Check if subject supports indexing. */
	if (!dt_result.refer_to_index) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
		une_result_free(subject);
		return une_result_create(UNE_RT_ERROR);
	}
	assert(dt_result.is_valid_index);
	
	/* Evaluate index. */
	une_result index = une_result_dereference(une_interpret(node->content.branch.b));
	if (index.type == UNE_RT_ERROR) {
		une_result_free(subject);
		return index;
	}
	
	/* Check if provided index is valid. */
	if (!dt_result.is_valid_index(subject, index)) {
		felix->error = UNE_ERROR_SET(UNE_ET_INDEX, node->content.branch.b->pos);
		une_result_free(subject);
		une_result_free(index);
		return une_result_create(UNE_RT_ERROR);
	}
	
	/* Refer to index. */
	une_result result = dt_result.refer_to_index(subject, index);
	assert(result.type == UNE_RT_REFERENCE);
	
	/* If the subject was NOT a reference (i.e. we interpreted a literal), we need to dereference the retrieved data *now*, because the literal will be deleted upon completion of this function. */
	if (UNE_RESULT_TYPE_IS_DATA_TYPE(subject.type))
		result = une_result_dereference(result);
	
	une_result_free(subject);
	une_result_free(index);
	return result;
}

une_interpreter__(une_interpret_idx_seek_range)
{
	/* Evaluate subject. */
	une_result subject = une_interpret(node->content.branch.a);
	if (subject.type == UNE_RT_ERROR)
		return subject;
	
	/* Get applicable datatype. */
	une_datatype dt_result = UNE_DATATYPE_FOR_RESULT(subject);
	
	/* Check if subject supports referring to ranges. */
	if (!dt_result.refer_to_index) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
		une_result_free(subject);
		return une_result_create(UNE_RT_ERROR);
	}
	assert(dt_result.is_valid_range);
	
	/* Evaluate indices. */
	une_result begin = une_result_dereference(une_interpret(node->content.branch.b));
	if (begin.type == UNE_RT_ERROR) {
		une_result_free(subject);
		return begin;
	}
	une_result end = une_result_dereference(une_interpret(node->content.branch.c));
	if (end.type == UNE_RT_ERROR) {
		une_result_free(subject);
		une_result_free(begin);
		return end;
	}
	
	/* Check if provided range is valid. */
	if (!dt_result.is_valid_range(subject, begin, end)) {
		felix->error = UNE_ERROR_SET(UNE_ET_INDEX, node->content.branch.b->pos);
		une_result_free(subject);
		une_result_free(begin);
		une_result_free(end);
		return une_result_create(UNE_RT_ERROR);
	}
	
	/* Refer to range. */
	une_result result = dt_result.refer_to_range(subject, begin, end);
	assert(result.type == UNE_RT_REFERENCE);
	
	/* If the subject was NOT a reference (i.e. we interpreted a literal), we need to dereference the retrieved data *now*, because the literal will be deleted upon completion of this function. */
	if (UNE_RESULT_TYPE_IS_DATA_TYPE(subject.type))
		result = une_result_dereference(result);
	
	une_result_free(subject);
	une_result_free(begin);
	une_result_free(end);
	return result;
}

une_interpreter__(une_interpret_comparison, une_int (*comparator)(une_result, une_result))
{
	/* Evaluate left branch. */
	une_node_type left_wrapped_as = UNE_NT_none__;
	une_result left = une_result_dereference(une_interpret(une_node_unwrap_any_or_all(node->content.branch.a, &left_wrapped_as)));
	if (left.type == UNE_RT_ERROR)
		return left;
	if (left_wrapped_as != UNE_NT_none__ && !UNE_DATATYPE_FOR_RESULT(left).refer_to_index) {
		une_result_free(left);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.a->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	if (left_wrapped_as == UNE_NT_none__)
		left = une_result_wrap_in_list(left);
	une_datatype dt_left = UNE_DATATYPE_FOR_RESULT(left);
	assert(dt_left.refer_to_index);
	assert(dt_left.get_len);
	size_t left_length = dt_left.get_len(left);
	
	/* Evaluate right branch. */
	une_node_type right_wrapped_as = UNE_NT_none__;
	une_result right = une_result_dereference(une_interpret(une_node_unwrap_any_or_all(node->content.branch.b, &right_wrapped_as)));
	if (right.type == UNE_RT_ERROR) {
		une_result_free(left);
		return right;
	}
	if (right_wrapped_as != UNE_NT_none__ && !UNE_DATATYPE_FOR_RESULT(right).refer_to_index) {
		une_result_free(left);
		une_result_free(right);
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->content.branch.b->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	if (right_wrapped_as == UNE_NT_none__)
		right = une_result_wrap_in_list(right);
	une_datatype dt_right = UNE_DATATYPE_FOR_RESULT(right);
	assert(dt_right.refer_to_index);
	assert(dt_right.get_len);
	size_t right_length = dt_right.get_len(right);
	
	/* Compare. */
	bool type_error = false;
	une_int complete_matches = 0;
	une_result i = une_result_create(UNE_RT_INT);
	une_result j = une_result_create(UNE_RT_INT);
	for (i.value._int=0; i.value._int<(une_int)left_length; i.value._int++) {
		une_int partial_matches = 0;
		for (j.value._int=0; j.value._int<(une_int)right_length; j.value._int++) {
			une_result left_element = une_result_dereference(dt_left.refer_to_index(left, i));
			une_result right_element = une_result_dereference(dt_right.refer_to_index(right, j));
			une_int match = comparator(left_element, right_element);
			une_result_free(left_element);
			une_result_free(right_element);
			if (match == -1) {
				type_error = true;
				break;
			}
			partial_matches += match;
		}
		if (type_error)
			break;
		if (right_wrapped_as == UNE_NT_ALL)
			complete_matches += partial_matches >= (une_int)right_length;
		else
			complete_matches += partial_matches > 0;
	}
	une_result_free(left);
	une_result_free(right);
	if (type_error) {
		felix->error = UNE_ERROR_SET(UNE_ET_TYPE, node->pos);
		return une_result_create(UNE_RT_ERROR);
	}
	une_int applies = 0;
	if (left_wrapped_as == UNE_NT_ALL) {
		applies = complete_matches >= (une_int)left_length;
	}
	else
		applies = complete_matches > 0;
	
	return (une_result){
		.type = UNE_RT_INT,
		.value._int = applies
	};
}
