/*
 * base.c - basic devicetree functions
 *
 * Copyright (c) 2012 Sascha Hauer <s.hauer@pengutronix.de>, Pengutronix
 *
 * based on Linux devicetree support
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <common.h>
#include <of.h>
#include <errno.h>
#include <libfdt.h>
#include <malloc.h>
#include <init.h>
#include <linux/ctype.h>

/**
 * struct alias_prop - Alias property in 'aliases' node
 * @link:	List node to link the structure in aliases_lookup list
 * @alias:	Alias property name
 * @np:		Pointer to device_node that the alias stands for
 * @id:		Index value from end of alias name
 * @stem:	Alias string without the index
 *
 * The structure represents one alias property of 'aliases' node as
 * an entry in aliases_lookup list.
 */
struct alias_prop {
	struct list_head link;
	const char *alias;
	struct device_node *np;
	int id;
	char stem[0];
};

static LIST_HEAD(aliases_lookup);

static LIST_HEAD(phandle_list);

static LIST_HEAD(allnodes);

struct device_node *root_node;

struct device_node *of_aliases;

int of_n_addr_cells(struct device_node *np)
{
	const __be32 *ip;

	do {
		if (np->parent)
			np = np->parent;
		ip = of_get_property(np, "#address-cells", NULL);
		if (ip)
			return be32_to_cpup(ip);
	} while (np->parent);
	/* No #address-cells property for the root node */
	return OF_ROOT_NODE_ADDR_CELLS_DEFAULT;
}
EXPORT_SYMBOL(of_n_addr_cells);

int of_n_size_cells(struct device_node *np)
{
	const __be32 *ip;

	do {
		if (np->parent)
			np = np->parent;
		ip = of_get_property(np, "#size-cells", NULL);
		if (ip)
			return be32_to_cpup(ip);
	} while (np->parent);
	/* No #size-cells property for the root node */
	return OF_ROOT_NODE_SIZE_CELLS_DEFAULT;
}
EXPORT_SYMBOL(of_n_size_cells);

static void of_bus_default_count_cells(struct device_node *dev,
				       int *addrc, int *sizec)
{
	if (addrc)
		*addrc = of_n_addr_cells(dev);
	if (sizec)
		*sizec = of_n_size_cells(dev);
}

void of_bus_count_cells(struct device_node *dev,
			int *addrc, int *sizec)
{
	of_bus_default_count_cells(dev, addrc, sizec);
}

struct property *of_find_property(const struct device_node *node, const char *name)
{
	struct property *p;

	list_for_each_entry(p, &node->properties, list)
		if (!strcmp(p->name, name))
			return p;
	return NULL;
}
EXPORT_SYMBOL(of_find_property);

static void of_alias_add(struct alias_prop *ap, struct device_node *np,
			 int id, const char *stem, int stem_len)
{
	ap->np = np;
	ap->id = id;
	strncpy(ap->stem, stem, stem_len);
	ap->stem[stem_len] = 0;
	list_add_tail(&ap->link, &aliases_lookup);
	pr_debug("adding DT alias:%s: stem=%s id=%i node=%s\n",
		 ap->alias, ap->stem, ap->id, np->full_name);
}

/**
 * of_alias_scan - Scan all properties of 'aliases' node
 *
 * The function scans all the properties of 'aliases' node and populates
 * the global lookup table with the properties.  It returns the
 * number of alias_prop found, or error code in error case.
 */
void of_alias_scan(void)
{
	struct property *pp;

	of_aliases = of_find_node_by_path("/aliases");
	if (!of_aliases)
		return;

	list_for_each_entry(pp, &of_aliases->properties, list) {
		const char *start = pp->name;
		const char *end = start + strlen(start);
		struct device_node *np;
		struct alias_prop *ap;
		int id, len;

		/* Skip those we do not want to proceed */
		if (!strcmp(pp->name, "name") ||
		    !strcmp(pp->name, "phandle") ||
		    !strcmp(pp->name, "linux,phandle"))
			continue;

		np = of_find_node_by_path(pp->value);
		if (!np)
			continue;

		/* walk the alias backwards to extract the id and work out
		 * the 'stem' string */
		while (isdigit(*(end-1)) && end > start)
			end--;
		len = end - start;

		id = simple_strtol(end, 0, 10);
		if (id < 0)
			continue;

		/* Allocate an alias_prop with enough space for the stem */
		ap = xzalloc(sizeof(*ap) + len + 1);
		if (!ap)
			continue;
		ap->alias = start;
		of_alias_add(ap, np, id, start, len);
	}
}

/**
 * of_alias_get_id - Get alias id for the given device_node
 * @np:		Pointer to the given device_node
 * @stem:	Alias stem of the given device_node
 *
 * The function travels the lookup table to get alias id for the given
 * device_node and alias stem.  It returns the alias id if find it.
 */
int of_alias_get_id(struct device_node *np, const char *stem)
{
	struct alias_prop *app;
	int id = -ENODEV;

	list_for_each_entry(app, &aliases_lookup, link) {
		if (strcmp(app->stem, stem) != 0)
			continue;

		if (np == app->np) {
			id = app->id;
			break;
		}
	}

	return id;
}
EXPORT_SYMBOL_GPL(of_alias_get_id);

u64 of_translate_address(struct device_node *node, const __be32 *in_addr)
{
	struct property *p;
	u64 addr = be32_to_cpu(*in_addr);

	while (1) {
		int na, nc;

		if (!node->parent)
			return addr;

		node = node->parent;
		p = of_find_property(node, "ranges");
		if (!p && node->parent)
			return OF_BAD_ADDR;
		of_bus_count_cells(node, &na, &nc);
		if (na != 1 || nc != 1) {
			printk("%s: #size-cells != 1 or #address-cells != 1 "
					"currently not supported\n", node->name);
			return OF_BAD_ADDR;
		}
	}
}
EXPORT_SYMBOL(of_translate_address);

/*
 * of_find_node_by_phandle - Find a node given a phandle
 * @handle:    phandle of the node to find
 */
struct device_node *of_find_node_by_phandle(phandle phandle)
{
	struct device_node *node;

	list_for_each_entry(node, &phandle_list, phandles)
		if (node->phandle == phandle)
			return node;
	return NULL;
}
EXPORT_SYMBOL(of_find_node_by_phandle);

/*
 * Find a property with a given name for a given node
 * and return the value.
 */
const void *of_get_property(const struct device_node *np, const char *name,
			 int *lenp)
{
	struct property *pp = of_find_property(np, name);

	if (!pp)
		return NULL;

	if (lenp)
		*lenp = pp->length;

	return pp ? pp->value : NULL;
}
EXPORT_SYMBOL(of_get_property);

/** Checks if the given "compat" string matches one of the strings in
 * the device's "compatible" property
 */
int of_device_is_compatible(const struct device_node *device,
		const char *compat)
{
	const char *cp;
	int cplen, l;

	cp = of_get_property(device, "compatible", &cplen);
	if (cp == NULL)
		return 0;
	while (cplen > 0) {
		if (strcmp(cp, compat) == 0)
			return 1;
		l = strlen(cp) + 1;
		cp += l;
		cplen -= l;
	}

	return 0;
}
EXPORT_SYMBOL(of_device_is_compatible);

int of_match(struct device_d *dev, struct driver_d *drv)
{
	struct of_device_id *id;

	id = drv->of_compatible;

	while (id->compatible) {
		if (of_device_is_compatible(dev->device_node, id->compatible) == 1) {
			dev->of_id_entry = id;
			return 0;
		}
		id++;
	}

	return 1;
}
EXPORT_SYMBOL(of_match);

/**
 * of_property_read_u32_array - Find and read an array of 32 bit integers
 * from a property.
 *
 * @np:		device node from which the property value is to be read.
 * @propname:	name of the property to be searched.
 * @out_value:	pointer to return value, modified only if return value is 0.
 *
 * Search for a property in a device node and read 32-bit value(s) from
 * it. Returns 0 on success, -EINVAL if the property does not exist,
 * -ENODATA if property does not have a value, and -EOVERFLOW if the
 * property data isn't large enough.
 *
 * The out_value is modified only if a valid u32 value can be decoded.
 */
int of_property_read_u32_array(const struct device_node *np,
			       const char *propname, u32 *out_values,
			       size_t sz)
{
	struct property *prop = of_find_property(np, propname);
	const __be32 *val;

	if (!prop)
		return -EINVAL;
	if (!prop->value)
		return -ENODATA;
	if ((sz * sizeof(*out_values)) > prop->length)
		return -EOVERFLOW;

	val = prop->value;
	while (sz--)
		*out_values++ = be32_to_cpup(val++);
	return 0;
}
EXPORT_SYMBOL_GPL(of_property_read_u32_array);

/**
 * of_parse_phandles_with_args - Find a node pointed by phandle in a list
 * @np:		pointer to a device tree node containing a list
 * @list_name:	property name that contains a list
 * @cells_name:	property name that specifies phandles' arguments count
 * @index:	index of a phandle to parse out
 * @out_node:	optional pointer to device_node struct pointer (will be filled)
 * @out_args:	optional pointer to arguments pointer (will be filled)
 *
 * This function is useful to parse lists of phandles and their arguments.
 * Returns 0 on success and fills out_node and out_args, on error returns
 * appropriate errno value.
 *
 * Example:
 *
 * phandle1: node1 {
 * 	#list-cells = <2>;
 * }
 *
 * phandle2: node2 {
 * 	#list-cells = <1>;
 * }
 *
 * node3 {
 * 	list = <&phandle1 1 2 &phandle2 3>;
 * }
 *
 * To get a device_node of the `node2' node you may call this:
 * of_parse_phandles_with_args(node3, "list", "#list-cells", 2, &node2, &args);
 */
int of_parse_phandles_with_args(struct device_node *np, const char *list_name,
				const char *cells_name, int index,
				struct device_node **out_node,
				const void **out_args)
{
	int ret = -EINVAL;
	const __be32 *list;
	const __be32 *list_end;
	int size;
	int cur_index = 0;
	struct device_node *node = NULL;
	const void *args = NULL;

	list = of_get_property(np, list_name, &size);
	if (!list) {
		ret = -ENOENT;
		goto err0;
	}
	list_end = list + size / sizeof(*list);

	while (list < list_end) {
		const __be32 *cells;
		phandle phandle;

		phandle = be32_to_cpup(list++);
		args = list;

		/* one cell hole in the list = <>; */
		if (!phandle)
			goto next;

		node = of_find_node_by_phandle(phandle);
		if (!node) {
			pr_debug("%s: could not find phandle %d\n",
				 np->full_name, phandle);
			goto err0;
		}

		cells = of_get_property(node, cells_name, &size);
		if (!cells || size != sizeof(*cells)) {
			pr_debug("%s: could not get %s for %s\n",
				 np->full_name, cells_name, node->full_name);
			goto err1;
		}

		list += be32_to_cpup(cells);
		if (list > list_end) {
			pr_debug("%s: insufficient arguments length\n",
				 np->full_name);
			goto err1;
		}
next:
		if (cur_index == index)
			break;

		node = NULL;
		args = NULL;
		cur_index++;
	}

	if (!node) {
		/*
		 * args w/o node indicates that the loop above has stopped at
		 * the 'hole' cell. Report this differently.
		 */
		if (args)
			ret = -EEXIST;
		else
			ret = -ENOENT;
		goto err0;
	}

	if (out_node)
		*out_node = node;
	if (out_args)
		*out_args = args;

	return 0;
err1:
err0:
	pr_debug("%s failed with status %d\n", __func__, ret);
	return ret;
}
EXPORT_SYMBOL(of_parse_phandles_with_args);

/**
 * of_machine_is_compatible - Test root of device tree for a given compatible value
 * @compat: compatible string to look for in root node's compatible property.
 *
 * Returns true if the root node has the given value in its
 * compatible property.
 */
int of_machine_is_compatible(const char *compat)
{
	if (!root_node)
		return 0;

	return of_device_is_compatible(root_node, compat);
}
EXPORT_SYMBOL(of_machine_is_compatible);

/**
 *	of_find_node_by_path - Find a node matching a full OF path
 *	@path:	The full path to match
 *
 *	Returns a node pointer with refcount incremented, use
 *	of_node_put() on it when done.
 */
struct device_node *of_find_node_by_path(const char *path)
{
	struct device_node *np;

	list_for_each_entry(np, &allnodes, list) {
		if (np->full_name && (strcmp(np->full_name, path) == 0))
			break;
	}
	return np;
}
EXPORT_SYMBOL(of_find_node_by_path);

struct device_node *of_get_root_node(void)
{
	return root_node;
}

static int of_node_disabled(struct device_node *node)
{
	struct property *p;

	p = of_find_property(node, "status");
	if (p) {
		if (!strcmp("disabled", p->value))
			return 1;
	}
	return 0;
}

void of_print_nodes(struct device_node *node, int indent)
{
	struct device_node *n;
	struct property *p;
	int i;

	if (!node)
		return;

	if (of_node_disabled(node))
		return;

	for (i = 0; i < indent; i++)
		printf("\t");

	printf("%s%s\n", node->name, node->name ? " {" : "{");

	list_for_each_entry(p, &node->properties, list) {
		for (i = 0; i < indent + 1; i++)
			printf("\t");
		printf("%s: ", p->name);
		of_print_property(p->value, p->length);
		printf("\n");
	}

	list_for_each_entry(n, &node->children, parent_list) {
		of_print_nodes(n, indent + 1);
	}

	for (i = 0; i < indent; i++)
		printf("\t");
	printf("};\n");
}

static struct device_node *new_device_node(struct device_node *parent)
{
	struct device_node *node;

	node = xzalloc(sizeof(*node));
	node->parent = parent;
	if (parent)
		list_add_tail(&node->parent_list, &parent->children);

	INIT_LIST_HEAD(&node->children);
	INIT_LIST_HEAD(&node->properties);

	return node;
}

static struct property *new_property(struct device_node *node, const char *name,
		const void *data, int len)
{
	struct property *prop;

	prop = xzalloc(sizeof(*prop));

	prop->name = strdup(name);
	prop->length = len;
	prop->value = xzalloc(len);
	memcpy(prop->value, data, len);

	list_add_tail(&prop->list, &node->properties);

	return prop;
}

static struct device_d *add_of_device(struct device_node *node)
{
	struct device_d *dev;
	char *name, *at;
	const struct property *cp;

	if (of_node_disabled(node))
		return NULL;

	cp = of_get_property(node, "compatible", NULL);
	if (!cp)
		return NULL;

	dev = xzalloc(sizeof(*dev));

	name = xstrdup(node->name);
	at = strchr(name, '@');
	if (at) {
		*at = 0;
		snprintf(dev->name, MAX_DRIVER_NAME, "%s.%s", at + 1, name);
	} else {
		strncpy(dev->name, node->name, MAX_DRIVER_NAME);
	}

	dev->id = DEVICE_ID_SINGLE;
	dev->resource = node->resource;
	dev->num_resources = 1;
	dev->device_node = node;
	node->device = dev;

	debug("register device 0x%08x\n", node->resource[0].start);

	platform_device_register(dev);

	free(name);

	return dev;
}
EXPORT_SYMBOL(add_of_device);

static int add_of_device_resource(struct device_node *node)
{
	struct property *reg;
	u64 address, size;
	struct resource *res;
	struct device_d *dev;
	phandle phandle;
	int ret;

	ret = of_property_read_u32(node, "phandle", &phandle);
	if (!ret) {
		node->phandle = phandle;
		list_add_tail(&node->phandles, &phandle_list);
	}

	reg = of_find_property(node, "reg");
	if (!reg)
		return -ENODEV;

	address = of_translate_address(node, reg->value);
	if (address == OF_BAD_ADDR)
		return -EINVAL;

	size = be32_to_cpu(((u32 *)reg->value)[1]);

	/*
	 * A device may already be registered as platform_device.
	 * Instead of registering the same device again, just
	 * add this node to the existing device.
	 */
	for_each_device(dev) {
		if (!dev->resource)
			continue;
		if (dev->resource->start == address) {
			debug("connecting %s to %s\n", node->name, dev_name(dev));
			node->device = dev;
			dev->device_node = node;
			node->resource = dev->resource;
			return 0;
		}
	}

	res = xzalloc(sizeof(*res));
	res->start = address;
	res->end = address + size - 1;
	res->flags = IORESOURCE_MEM;

	node->resource = res;

	add_of_device(node);

	return 0;
}

void of_free(struct device_node *node)
{
	struct device_node *n, *nt;
	struct property *p, *pt;

	if (!node)
		return;

	list_for_each_entry_safe(p, pt, &node->properties, list) {
		list_del(&p->list);
		free(p->name);
		free(p->value);
		free(p);
	}

	list_for_each_entry_safe(n, nt, &node->children, parent_list) {
		of_free(n);
	}

	if (node->parent)
		list_del(&node->parent_list);

	if (node->device)
		node->device->device_node = NULL;
	else
		free(node->resource);

	free(node->name);
	free(node->full_name);
	free(node);
}

static void __of_probe(struct device_node *node)
{
	struct device_node *n;

	if (node->device)
		return;

	add_of_device_resource(node);

	list_for_each_entry(n, &node->children, parent_list)
		__of_probe(n);
}

int of_probe(void)
{
	if(!root_node)
		return -ENODEV;

	__of_probe(root_node);

	return 0;
}

/*
 * Parse a flat device tree binary blob and store it in the barebox
 * internal tree format,
 */
int of_parse_dtb(struct fdt_header *fdt)
{
	const void *nodep;	/* property node pointer */
	int  nodeoffset;	/* node offset from libfdt */
	int  nextoffset;	/* next node offset from libfdt */
	uint32_t tag;		/* tag */
	int  len;		/* length of the property */
	int  level = 0;		/* keep track of nesting level */
	const struct fdt_property *fdt_prop;
	const char *pathp;
	int depth = 10000;
	struct device_node *node = NULL;
	char buf[1024];
	int ret;

	if (root_node)
		return -EBUSY;

	nodeoffset = fdt_path_offset(fdt, "/");
	if (nodeoffset < 0) {
		/*
		 * Not found or something else bad happened.
		 */
		printf ("libfdt fdt_path_offset() returned %s\n",
			fdt_strerror(nodeoffset));
		return -EINVAL;
	}

	while (1) {
		tag = fdt_next_tag(fdt, nodeoffset, &nextoffset);
		switch (tag) {
		case FDT_BEGIN_NODE:
			pathp = fdt_get_name(fdt, nodeoffset, NULL);

			if (pathp == NULL)
				pathp = "/* NULL pointer error */";

			ret = fdt_get_path(fdt, nodeoffset, buf, 1024);
			if (ret)
				return -EINVAL;

			node = new_device_node(node);
			if (!node->parent)
				root_node = node;
			node->full_name = xstrdup(buf);
			node->name = xstrdup(pathp);
			list_add_tail(&node->list, &allnodes);
			break;
		case FDT_END_NODE:
			node = node->parent;
			break;
		case FDT_PROP:
			fdt_prop = fdt_offset_ptr(fdt, nodeoffset,
					sizeof(*fdt_prop));
			pathp    = fdt_string(fdt,
					fdt32_to_cpu(fdt_prop->nameoff));
			len      = fdt32_to_cpu(fdt_prop->len);
			nodep    = fdt_prop->data;
			new_property(node, pathp, nodep, len);
			break;
		case FDT_NOP:
			break;
		case FDT_END:
			of_alias_scan();
			return 0;
		default:
			if (level <= depth)
				printf("Unknown tag 0x%08X\n", tag);
			return -EINVAL;
		}
		nodeoffset = nextoffset;
	}

	return 0;
}
