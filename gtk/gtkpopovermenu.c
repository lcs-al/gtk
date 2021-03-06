/* GTK - The GIMP Toolkit
 * Copyright © 2014 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "gtkpopovermenu.h"
#include "gtkstack.h"
#include "gtkstylecontext.h"
#include "gtkintl.h"
#include "gtkpopoverprivate.h"


/**
 * SECTION:gtkpopovermenu
 * @Short_description: Popovers to use as menus
 * @Title: GtkPopoverMenu
 *
 * GtkPopoverMenu is a subclass of #GtkPopover that treats its
 * children like menus and allows switching between them. It is
 * meant to be used primarily together with #GtkModelButton, but
 * any widget can be used, such as #GtkSpinButton or #GtkScale.
 * In this respect, GtkPopoverMenu is more flexible than popovers
 * that are created from a #GMenuModel with gtk_popover_new_from_model().
 *
 * To add a child as a submenu, use gtk_popover_menu_add_submenu().
 * To let the user open this submenu, add a #GtkModelButton whose
 * #GtkModelButton:menu-name property is set to the name you've given
 * to the submenu.
 *
 * To add a named submenu in a ui file, set the #GtkWidget:name property
 * of the widget that you are adding as a child of the popover menu.
 *
 * By convention, the first child of a submenu should be a #GtkModelButton
 * to switch back to the parent menu. Such a button should use the
 * #GtkModelButton:inverted and #GtkModelButton:centered properties
 * to achieve a title-like appearance and place the submenu indicator
 * at the opposite side. To switch back to the main menu, use "main"
 * as the menu name.
 *
 * # Example
 *
 * |[
 * <object class="GtkPopoverMenu">
 *   <child>
 *     <object class="GtkBox">
 *       <property name="visible">True</property>
 *       <property name="margin">10</property>
 *       <child>
 *         <object class="GtkModelButton">
 *           <property name="visible">True</property>
 *           <property name="action-name">win.frob</property>
 *           <property name="text" translatable="yes">Frob</property>
 *         </object>
 *       </child>
 *       <child>
 *         <object class="GtkModelButton">
 *           <property name="visible">True</property>
 *           <property name="menu-name">more</property>
 *           <property name="text" translatable="yes">More</property>
 *         </object>
 *       </child>
 *     </object>
 *   </child>
 *   <child>
 *     <object class="GtkBox">
 *       <property name="visible">True</property>
 *       <property name="margin">10</property>
 *       <property name="name">more</property>
 *       <child>
 *         <object class="GtkModelButton">
 *           <property name="visible">True</property>
 *           <property name="action-name">win.foo</property>
 *           <property name="text" translatable="yes">Foo</property>
 *         </object>
 *       </child>
 *       <child>
 *         <object class="GtkModelButton">
 *           <property name="visible">True</property>
 *           <property name="action-name">win.bar</property>
 *           <property name="text" translatable="yes">Bar</property>
 *         </object>
 *       </child>
 *     </object>
 *   </child>
 * </object>
 * ]|
 *
 * # CSS Nodes
 *
 * #GtkPopoverMenu is just a subclass of #GtkPopover that adds
 * custom content to it, therefore it has the same CSS nodes.
 * It is one of the cases that add a .menu style class to the
 * popover's contents node.
 */

struct _GtkPopoverMenu
{
  GtkPopover parent_instance;
};

enum {
  PROP_VISIBLE_SUBMENU = 1
};

G_DEFINE_TYPE (GtkPopoverMenu, gtk_popover_menu, GTK_TYPE_POPOVER)

static void
visible_submenu_changed (GObject        *object,
                         GParamSpec     *pspec,
                         GtkPopoverMenu *popover)
{
  g_object_notify (G_OBJECT (popover), "visible-submenu");
}

static void
gtk_popover_menu_init (GtkPopoverMenu *popover)
{
  GtkWidget *stack;
  GtkStyleContext *style_context;

  stack = gtk_stack_new ();
  gtk_stack_set_vhomogeneous (GTK_STACK (stack), FALSE);
  gtk_stack_set_transition_type (GTK_STACK (stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
  gtk_stack_set_interpolate_size (GTK_STACK (stack), TRUE);
  gtk_container_add (GTK_CONTAINER (popover), stack);
  g_signal_connect (stack, "notify::visible-child-name",
                    G_CALLBACK (visible_submenu_changed), popover);

  style_context = gtk_widget_get_style_context (gtk_popover_get_contents_widget (GTK_POPOVER (popover)));
  gtk_style_context_add_class (style_context, GTK_STYLE_CLASS_MENU);
}

static void
gtk_popover_menu_map (GtkWidget *widget)
{
  GTK_WIDGET_CLASS (gtk_popover_menu_parent_class)->map (widget);
  gtk_popover_menu_open_submenu (GTK_POPOVER_MENU (widget), "main");
}

static void
gtk_popover_menu_unmap (GtkWidget *widget)
{
  gtk_popover_menu_open_submenu (GTK_POPOVER_MENU (widget), "main");
  GTK_WIDGET_CLASS (gtk_popover_menu_parent_class)->unmap (widget);
}

static void
gtk_popover_menu_add (GtkContainer *container,
                      GtkWidget    *child)
{
  GtkWidget *stack;

  stack = gtk_bin_get_child (GTK_BIN (container));

  if (stack == NULL)
    {
      GTK_CONTAINER_CLASS (gtk_popover_menu_parent_class)->add (container, child);
    }
  else
    {
      const char *name;

      if (gtk_widget_get_name (child))
        name = gtk_widget_get_name (child);
      else if (gtk_stack_get_child_by_name (GTK_STACK (stack), "main"))
        name = "submenu";
      else
        name = "main";

      gtk_popover_menu_add_submenu (GTK_POPOVER_MENU (container), child, name);
    }
}

static void
gtk_popover_menu_remove (GtkContainer *container,
                         GtkWidget    *child)
{
  GtkWidget *stack;

  stack = gtk_bin_get_child (GTK_BIN (container));

  if (child == stack)
    GTK_CONTAINER_CLASS (gtk_popover_menu_parent_class)->remove (container, child);
  else
    gtk_container_remove (GTK_CONTAINER (stack), child);
}

static void
gtk_popover_menu_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  GtkWidget *stack;

  stack = gtk_bin_get_child (GTK_BIN (object));

  switch (property_id)
    {
    case PROP_VISIBLE_SUBMENU:
      g_value_set_string (value, gtk_stack_get_visible_child_name (GTK_STACK (stack)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gtk_popover_menu_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  GtkWidget *stack;

  stack = gtk_bin_get_child (GTK_BIN (object));

  switch (property_id)
    {
    case PROP_VISIBLE_SUBMENU:
      gtk_stack_set_visible_child_name (GTK_STACK (stack), g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gtk_popover_menu_class_init (GtkPopoverMenuClass *klass)
{
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = gtk_popover_menu_set_property;
  object_class->get_property = gtk_popover_menu_get_property;

  widget_class->map = gtk_popover_menu_map;
  widget_class->unmap = gtk_popover_menu_unmap;

  container_class->add = gtk_popover_menu_add;
  container_class->remove = gtk_popover_menu_remove;

  g_object_class_install_property (object_class,
                                   PROP_VISIBLE_SUBMENU,
                                   g_param_spec_string ("visible-submenu",
                                                        P_("Visible submenu"),
                                                        P_("The name of the visible submenu"),
                                                        NULL,
                                                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

/**
 * gtk_popover_menu_new:
 *
 * Creates a new popover menu.
 *
 * Returns: a new #GtkPopoverMenu
 */
GtkWidget *
gtk_popover_menu_new (void)
{
  return g_object_new (GTK_TYPE_POPOVER_MENU, NULL);
}

/**
 * gtk_popover_menu_open_submenu:
 * @popover: a #GtkPopoverMenu
 * @name: the name of the menu to switch to
 *
 * Opens a submenu of the @popover. The @name
 * must be one of the names given to the submenus
 * of @popover with #GtkPopoverMenu:submenu, or
 * "main" to switch back to the main menu.
 *
 * #GtkModelButton will open submenus automatically
 * when the #GtkModelButton:menu-name property is set,
 * so this function is only needed when you are using
 * other kinds of widgets to initiate menu changes.
 */
void
gtk_popover_menu_open_submenu (GtkPopoverMenu *popover,
                               const gchar    *name)
{
  GtkWidget *stack;

  g_return_if_fail (GTK_IS_POPOVER_MENU (popover));

  stack = gtk_bin_get_child (GTK_BIN (popover));
  gtk_stack_set_visible_child_name (GTK_STACK (stack), name);
}

/**
 * gtk_popover_menu_add_submenu:
 * @popover: a #GtkPopoverMenu
 * @submenu: a widget to add as submenu
 * @name: the name for the submenu
 *
 * Adds a submenu to the popover menu.
 */
void
gtk_popover_menu_add_submenu (GtkPopoverMenu *popover,
                              GtkWidget      *submenu,
                              const char     *name)
{
  GtkWidget *stack;

  stack = gtk_bin_get_child (GTK_BIN (popover));

  gtk_stack_add_named (GTK_STACK (stack), submenu, name);
} 
