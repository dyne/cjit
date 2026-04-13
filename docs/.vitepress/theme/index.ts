// https://vitepress.dev/guide/custom-theme
import { h } from 'vue'
import type { Theme } from 'vitepress'
import DefaultTheme from 'vitepress/theme'
import DocBrand from './components/DocBrand.vue'
import DyneBrand from './components/DyneBrand.vue'
import './style.css'

export default {
  extends: DefaultTheme,
  Layout: () => {
    return h(DefaultTheme.Layout, null, {
      // https://vitepress.dev/guide/extending-default-theme#layout-slots
      'nav-bar-title-before': () => h(DyneBrand, { variant: 'nav' }),
//      'doc-before': () => h(DocBrand)
    })
  },
  enhanceApp({ app, router, siteData }) {
    // ...
  }
} satisfies Theme
