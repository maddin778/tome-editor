﻿// --------------------------------------------------------------------------------------------------------------------
// <copyright file="ControlUtils.cs" company="Tome">
//   Copyright (c) Tome. All rights reserved.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

namespace Tome.Util
{
    using System.Collections.Generic;
    using System.Windows.Controls;
    using System.Windows.Controls.Primitives;
    using System.Windows.Data;

    using Tome.Core.Conversion;
    using Tome.Core.Validation;
    using Tome.Fields.Controls;
    using Tome.Model.Data;
    using Tome.Model.Fields;

    using Xceed.Wpf.Toolkit;

    public class ControlUtils
    {
        #region Public Methods and Operators

        public static Control CreateValueControl(object viewModel, string bindingPath, FieldType fieldType)
        {
            return CreateValueControl(viewModel, bindingPath, fieldType, null);
        }

        public static Control CreateValueControl(object viewModel, string bindingPath, FieldType fieldType, object data)
        {
            Control control;

            // Create binding.
            var binding = new Binding(bindingPath);
            binding.Source = viewModel;

            // Create control.
            switch (fieldType)
            {
                case FieldType.Boolean:
                    control = new CheckBox();
                    control.SetBinding(ToggleButton.IsCheckedProperty, binding);
                    return control;

                case FieldType.Color:
                    control = new ColorCanvas();
                    control.SetBinding(ColorCanvas.SelectedColorProperty, binding);
                    return control;

                case FieldType.Float:
                    control = new TextBox();
                    binding.ValidationRules.Add(new CanConvertToTypeValidationRule(typeof(float)));
                    binding.Converter = new FieldValueConverter(typeof(float));
                    control.SetBinding(TextBox.TextProperty, binding);
                    return control;

                case FieldType.Int:
                    control = new TextBox();
                    binding.ValidationRules.Add(new CanConvertToTypeValidationRule(typeof(int)));
                    binding.Converter = new FieldValueConverter(typeof(int));
                    control.SetBinding(TextBox.TextProperty, binding);
                    return control;

                case FieldType.Reference:
                    control = new ComboBox();
                    control.SetBinding(Selector.SelectedItemProperty, binding);

                    if (data != null)
                    {
                        var comboBox = (ComboBox)control;
                        var options = (IEnumerable<string>)data;

                        foreach (var option in options)
                        {
                            comboBox.Items.Add(option);
                        }
                    }

                    return control;

                case FieldType.String:
                    control = new TextBox();
                    binding.ValidationRules.Add(new CanConvertToTypeValidationRule(typeof(string)));
                    binding.Converter = new FieldValueConverter(typeof(string));
                    control.SetBinding(TextBox.TextProperty, binding);
                    return control;

                case FieldType.Vector3F:
                    control = new Vector3FControl();
                    control.SetBinding(Vector3FControl.VectorProperty, binding);
                    return control;
            }

            return null;
        }

        /// <summary>
        ///   "It's a real pain for some strange reason."
        /// </summary>
        /// <see cref="http://stackoverflow.com/questions/413890/how-to-programmatically-select-an-item-in-a-wpf-treeview" />
        /// <see cref="http://blog.quantumbitdesigns.com/2008/07/22/programmatically-selecting-an-item-in-a-treeview/" />
        /// <see cref="http://alookonthecode.blogspot.de/2011/05/pogrammatically-selecting-items-in-wpf.html" />
        /// <param name="parentContainer">Container holding the item to select.</param>
        /// <param name="itemToSelect">Item to select and focus.</param>
        /// <returns></returns>
        public static bool ExpandAndSelectItem(ItemsControl parentContainer, object itemToSelect)
        {
            foreach (var item in parentContainer.Items)
            {
                // Check current container.
                var currentContainer = parentContainer.ItemContainerGenerator.ContainerFromItem(item) as TreeViewItem;

                if (item == itemToSelect && currentContainer != null)
                {
                    // Found it!
                    currentContainer.IsSelected = true;
                    currentContainer.BringIntoView();
                    currentContainer.Focus();
                    return true;
                }

                if (currentContainer != null && currentContainer.Items.Count > 0)
                {
                    // Force children expansion.
                    currentContainer.IsExpanded = true;
                    currentContainer.UpdateLayout();

                    // Check children.
                    if (ExpandAndSelectItem(currentContainer, itemToSelect))
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        #endregion
    }
}