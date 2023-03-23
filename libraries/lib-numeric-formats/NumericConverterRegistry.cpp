/*  SPDX-License-Identifier: GPL-2.0-or-later */
/**********************************************************************

 Audacity: A Digital Audio Editor

 @file NumericConverterRegistry.cpp

 Dmitry Vedenko

 **********************************************************************/
#include "NumericConverterRegistry.h"

namespace
{
const auto PathStart = L"NumericConverterRegistry";

struct RegistryVisitor : public Registry::Visitor
{
   RegistryVisitor(
      NumericConverterRegistry::Visitor _visitor, NumericConverterType requestedType)
       : visitor { std::move(_visitor) }
       , requestedType { std::move(requestedType) }
   {
   }

   void BeginGroup(Registry::GroupItem& item, const Path&) override
   {
      auto concreteGroup = dynamic_cast<NumericConverterRegistryGroup*>(&item);

      mInMatchingGroup =
         concreteGroup != nullptr && concreteGroup->type == requestedType;
   }

   void EndGroup(Registry::GroupItem&, const Path&) override
   {
      mInMatchingGroup = false;
   }

   void Visit(Registry::SingleItem& item, const Path&) override
   {
      if (!mInMatchingGroup)
         return;

      auto concreteItem = dynamic_cast<NumericConverterRegistryItem*>(&item);

      if (concreteItem == nullptr)
         return;

      visitor(*concreteItem);
   }

   NumericConverterRegistry::Visitor visitor;
   const NumericConverterType requestedType;
   bool mInMatchingGroup { false };
};
}

 NumericConverterRegistryItem::NumericConverterRegistryItem(
   const Identifier& internalName, const NumericFormatSymbol& _symbol,
   NumericConverterFormatterFactory _factory)
    : SingleItem { internalName }
    , symbol { _symbol }
    , factory { std::move(_factory) }
{
 }

 NumericConverterRegistryItem::NumericConverterRegistryItem(
    const Identifier& internalName, const NumericFormatSymbol& _symbol,
    const TranslatableString& _fractionLabel,
    NumericConverterFormatterFactory _factory)
     : SingleItem { internalName }
     , symbol { _symbol }
     , fractionLabel { _fractionLabel }
     , factory { std::move(_factory) }
 {
 }

 NumericConverterRegistryItem::~NumericConverterRegistryItem()
 {
 }

Registry::GroupItem& NumericConverterRegistry::Registry()
{
   static Registry::TransparentGroupItem<> registry { PathStart };
   return registry;
}

void NumericConverterRegistry::Visit(NumericConverterType type, Visitor visitor)
{
   static Registry::OrderingPreferenceInitializer init {
      PathStart,
      { { L"", L"" } },
   };

   RegistryVisitor registryVisitor { std::move(visitor), type };

   Registry::TransparentGroupItem<> top { PathStart };
   Registry::Visit(registryVisitor, &top, &Registry());
}

const NumericConverterRegistryItem*
NumericConverterRegistry::Find(NumericConverterType type, const NumericFormatSymbol& symbol)
{
   const NumericConverterRegistryItem* result = nullptr;

   Visit(
      type,
      [&result, symbol](const NumericConverterRegistryItem& item)
      {
         if (item.symbol == symbol)
            result = &item;
      });

   return result;
}

NumericConverterRegistryGroup::~NumericConverterRegistryGroup()
{
}

bool NumericConverterRegistryGroup::Transparent() const
{
   return true;
}

NumericConverterItemRegistrator::NumericConverterItemRegistrator(
   const Registry::Placement& placement, Registry::BaseItemPtr pItem)
    : RegisteredItem { std::move(pItem), placement }
{
}

NUMERIC_FORMATS_API Registry::BaseItemPtr NumericConverterFormatterItem(
   const Identifier& functionId, const TranslatableString& label,
   NumericConverterFormatterFactory factory)
{
   return std::make_unique<NumericConverterRegistryItem>(
      functionId, label, std::move(factory));
}

NUMERIC_FORMATS_API Registry::BaseItemPtr NumericConverterFormatterItem(
   const Identifier& functionId, const TranslatableString& label,
   const TranslatableString& fractionLabel,
   NumericConverterFormatterFactory factory)
{
   return std::make_unique<NumericConverterRegistryItem>(
      functionId, label, fractionLabel, std::move(factory));
}
